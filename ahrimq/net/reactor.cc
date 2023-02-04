#include "net/reactor.h"

using namespace std::placeholders;  // _1, _2

namespace ahrimq {

uint64_t Reactor::next_conn_id_ = 1;

Reactor::Reactor(const std::string& ip, uint16_t port, uint32_t num)
    : num_loop_(num), rand_engine_(std::time(nullptr)) {
  if (num == 0) {
    num_loop_ = std::thread::hardware_concurrency();
  }
  // init eventloop
  if (!InitEventLoops()) {
    std::cerr << "can not init eventloops\n";
    exit(EXIT_FAILURE);
  }
  if (!InitAddr(ip, port)) {
    std::cerr << "can not create address\n";
    exit(EXIT_FAILURE);
  }
  if (!InitAcceptor()) {
    exit(EXIT_FAILURE);
  }
}

Reactor::~Reactor() {
  eventloops_[0]->epoller->DetachConn(acceptor_.get());
  acceptor_.reset();
  for (auto&& loop : eventloops_) {
    if (!loop->stopped) {
      loop->Stop();
    }
  }
  // destroy all connections
  mtx_.lock();
  conns_.clear();
  mtx_.unlock();
}

void Reactor::React() {
  // start background threads and start running
  for (size_t i = 0; i < num_loop_; i++) {
    worker_threads_.push_back(std::thread(
        [&](int index) {
          printf("Loop-%d started\n", index);
          this->eventloops_[index]->Loop();  // blocked for every thread
        },
        i));
  }
}

void Reactor::Wait() {
  for (auto&& wkr : worker_threads_) {
    if (wkr.joinable()) {
      wkr.join();
    }
  }
}

void Reactor::Stop() {
  for (auto&& loop : eventloops_) {
    loop->Stop();
  }
}

void Reactor::CloseConn(ReactorConn* conn) {
  if (conn != nullptr) {
    // conn->loop_->epoller->DetachConn(conn); and close(conn->fd_); already done in
    // ReactorConn::~ReactorConn
    std::string name = conn->name_;
    mtx_.lock();
    conns_.erase(conn->name_);
    mtx_.unlock();
    // FIXME reuse connection instances: if we actually reuse the connection
    // instance, we need to close(fd) and DetachConn(conn) manually
#ifdef AHRIMQ_DEBUG
    printf("TCP connection [%s] closed\n", name.c_str());
#endif
  }
}

bool Reactor::InitEventLoops() {
  eventloops_.reserve((size_t)num_loop_);
  for (size_t i = 0; i < num_loop_; i++) {
    try {
      eventloops_.push_back(std::make_shared<EventLoop>());
    } catch (std::exception& ex) {
      std::cerr << "Reactor::InitEventLoops failed due to " << ex.what()
                << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  return true;
}

bool Reactor::InitAddr(const std::string& ip, uint16_t port) {
  addr_ = std::make_shared<IPAddr4>(ip, port);
  return addr_ != nullptr;
}

bool Reactor::InitAcceptor() {
  int lfd = socket(AF_INET, SOCK_STREAM, 0);
  SetReuseAddress(lfd);
  SetReusePort(lfd);
  SetFdNonBlock(lfd);
  if (lfd == -1) {
    std::cerr << "create acceptor fd failed: " << strerror(errno) << std::endl;
    return false;
  }
  // bind acceptor fd to addr
  if (bind(lfd, addr_->GetAddr(), addr_->GetSockAddrLen()) == -1) {
    std::cerr << "bind acceptor address failed : " << strerror(errno) << std::endl;
    return false;
  }

  // construct new acceptor
  acceptor_ = std::make_shared<ReactorConn>(
      lfd, EPOLLIN, std::bind(&Reactor::Acceptor, this, _1, _2), nullptr,
      eventloops_[0].get(), "conn-acceptor");

  listen(lfd, SOMAXCONN);
  if (!eventloops_[0]->epoller->AttachConn(acceptor_.get())) {
    std::cerr << "can not attach acceptor to epoller\n";
    return false;
  }
  acceptor_->watched_ = true;
  return true;
}

// ATTENTION: this method works in single thread
void Reactor::Acceptor(ReactorConn* conn, bool& closed) {
  // accept incoming connection(session) and process
  IPAddr4 addr;
  socklen_t socklen = addr.GetSockAddrLen();
  int remote_fd = accept(acceptor_->fd_, addr.GetAddr(), &socklen);
  if (remote_fd != -1) {
    addr.SyncPort();
    char buf[64];
    memset(buf, 0, sizeof(buf));
    snprintf(buf, sizeof(buf), "*%s#%lu", addr.ToString().c_str(), next_conn_id_++);
    std::string newconn_name(buf, std::strlen(buf));
    // std::string newconn_name =
    // "*" + addr.ToString() + "#" + std::to_string(next_conn_id_++);
    auto selected_loop = EventLoopSelector();

    // FIXME: reuse conn instance from connection pool
    auto reader = [this](ReactorConn* incomming_conn, bool& closed) {
      this->Reader(incomming_conn, closed);
    };
    auto writer = [this](ReactorConn* incomming_conn, bool& closed) {
      this->Writer(incomming_conn, closed);
    };

    ReactorConnPtr newconn =
        std::make_shared<ReactorConn>(remote_fd, EPOLLIN | EPOLLONESHOT, reader,
                                      writer, selected_loop, newconn_name);
    if (newconn != nullptr) {
      SetFdNonBlock(remote_fd);
      // attach new session into epoll
      if (ev_accept_handler_ != nullptr) {
        // in accept handler, we should set conn's read_buf and write_buf pointer
        bool close_after = false;
        ev_accept_handler_(newconn.get(), close_after);
        if (close_after) {
          CloseConn(newconn.get());
        }
      }
      if (selected_loop->epoller->AttachConn(newconn.get())) {
        newconn->watched_ = true;
        mtx_.lock();
        conns_[newconn_name] = newconn;
        mtx_.unlock();
#ifdef AHRIMQ_DEBUG
        printf("TCP connection %s opened\n", newconn_name.c_str());
#endif
      }
    } else {
      // can not create connection instance
      // we need to close remote_fd
      close(remote_fd);
    }
  } else {
    // accept failed
    printf("can not accept due to %s\n", strerror(errno));
  }
}

// EPOLLIN handler
// ATTENTION!!: this method is called in multiple thread
// all we do in this method is to read fd and put data into conn->read_buf_
void Reactor::Reader(ReactorConn* conn, bool& closed) {
  int fd = conn->fd_;
  Buffer* rbuf = conn->read_buf_;
  if (rbuf == nullptr) {
    std::cerr << "[" << conn->GetName() << "] rbuf is nullptr, invalid status!!\n";
    return;
  }
  char tmpbuf[kNetReadBufSize] = {0};
  int rflag = 0;
  size_t n = FixedSizeReadToBuf(fd, tmpbuf, sizeof(tmpbuf), &rflag);
  if (n == 0 && rflag == READ_SOCKET_CLOSED) {
    // connection closed
    if (ev_close_handler_ != nullptr) {
      bool close_after = false;
      ev_close_handler_(conn, close_after);
      CloseConn(conn);
      closed = true;
      return;
    }
  } else {
    // normal read
    // FIXME: there exists a copy cost
    rbuf->Append(tmpbuf, n);
    if (ev_read_handler_ != nullptr) {
      bool close_after = false;
      ev_read_handler_(conn, rflag == READ_EOF_REACHED, close_after);
      if (close_after) {
        CloseConn(conn);
      }
    }
    if (conn->write_buf_->Size() > 0) {
      conn->SetMaskWrite();
      // every thread has its own epoller
      conn->loop_->epoller->ModifyConn(conn);
    }
  }
}

void Reactor::SendFileAndUpdate(ReactorConn* conn, int outfd) {
  int infd = conn->file_state_.fd_ready_;
  size_t offset = conn->file_state_.offset_;
  size_t target_len = conn->file_state_.target_size_ - offset;
  size_t file_sent_bytes = SendFile(infd, outfd, offset, target_len);
  // update file state
  conn->file_state_.offset_ += file_sent_bytes;
  conn->file_state_.target_size_ -= file_sent_bytes;
}

// ATTENTION: this method may be invoked in multiple threads
void Reactor::InvokeWriteDoneHandler(ReactorConn* conn, Buffer* wbuf) {
  conn->SetMaskRead();
  // every thread has its own epoller
  conn->loop_->epoller->ModifyConn(conn);
  wbuf->Reset();
  if (ev_write_handler_ != nullptr) {
    bool close_after = false;
    ev_write_handler_(conn, close_after);
    if (close_after) {
      CloseConn(conn);
    }
  }
}

// EPOLLOUT handler
// ATTENTION!!: this method is called in multiple thread;
// all we do in this method is to write out all bytes in conn->write_buf_
void Reactor::Writer(ReactorConn* conn, bool& closed) {
  int fd = conn->fd_;
  Buffer* wbuf = conn->write_buf_;
  if (wbuf == nullptr) {
    std::cerr << "[" << conn->GetName() << "] wbuf is nullptr, invalid status!!\n";
    return;
  }
  if (wbuf->Size() <= 0) {
    conn->SetMaskRead();
    // every thread has its own epoller
    conn->loop_->epoller->ModifyConn(conn);
    return;
  }
  size_t n = wbuf->Size();
  if (n == 0) {  // write buffer is all sent, by file is partially sent
    // check file is needed to send
    if (conn->FileNeedSending()) {
      // send file
      SendFileAndUpdate(conn, fd);
      if (conn->file_state_.target_size_ == 0) {
        // we have already send all write buffer data and file has been sent
        InvokeWriteDoneHandler(conn, wbuf);
        if (conn->file_state_.close_after_) {
          close(conn->file_state_.fd_ready_);
        }
      }
    }
  }
  size_t nbytes =
      FixedSizeWriteFromBuf(fd, static_cast<const char*>(wbuf->BeginReadIndex()), n);
  if (nbytes > 0) {
    if (nbytes == n) {
      // all bytes have been sent
      if (conn->FileNeedSending()) {
        // file sending is needed
        SendFileAndUpdate(conn, fd);
        if (conn->file_state_.target_size_ == 0) {
          // we have already send all write buffer data and file has been sent
          InvokeWriteDoneHandler(conn, wbuf);
          if (conn->file_state_.close_after_) {
            close(conn->file_state_.fd_ready_);
          }
        }
      } else {
        // we don't need to send file
        InvokeWriteDoneHandler(conn, wbuf);
      }
    } else {
      // write not fully sent, we have to consume partial content
      wbuf->ReaderIdxForward(nbytes);
    }
  }
  if (nbytes == 0 && wbuf->Size() != 0) {
    // we condiser this as an invalid state
    CloseConn(conn);
    closed = true;
  }
}

// TODO optimize the loop selection algorithm
EventLoop* Reactor::EventLoopSelector() {
  if (num_loop_ == 1) {
    return eventloops_[0].get();
  }
  uint64_t idx =
      std::uniform_int_distribution<uint64_t>(1, num_loop_ - 1)(rand_engine_);
#ifdef AHRIMQ_DEBUG
  printf("Assigned to eventloop-%lu\n", idx);
#endif
  return eventloops_[idx].get();
}

}  // namespace ahrimq