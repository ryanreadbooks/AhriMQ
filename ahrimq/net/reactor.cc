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
    loop->Stop();
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
          std::cout << "Loop-" << index << " started\n";
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

void Reactor::CloseConn(ReactorConn* conn) {
  if (conn != nullptr) {
    // conn->loop_->epoller->DetachConn(conn); // already done in
    // ReactorConn::~ReactorConn
    conn->read_buf_->Reset();
    conn->write_buf_->Reset();
    // close(conn->fd_);  // // already done in ReactorConn::~ReactorConn
    conns_.erase(conn->name_);
    // FIXME reuse connection instances: if we actually reuse the connection
    // instance, we need to close(fd) and DetachConn(conn) manually
  }
}

bool Reactor::InitEventLoops() {
  eventloops_.reserve((size_t)num_loop_);
  for (size_t i = 0; i < num_loop_; i++) {
    eventloops_.push_back(std::make_shared<EventLoop>());
    // TODO check success or not
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
    std::string conn_name = buf;
    auto selected_loop = EventLoopSelector();
    // FIXME: reuse conn instance from connection pool
    ReactorConnPtr conn = std::make_shared<ReactorConn>(
        remote_fd, EPOLLIN, std::bind(&Reactor::Reader, this, _1, _2),
        std::bind(&Reactor::Writer, this, _1, _2), selected_loop, conn_name);
    if (conn != nullptr) {
      SetFdNonBlock(remote_fd);
      // attach new session into epoll
      if (ev_accept_handler_ != nullptr) {
        // in accept handler, we should set conn's read_buf and write_buf pointer
        bool close_after = false;
        ev_accept_handler_(conn.get(), close_after);
      }
      if (selected_loop->epoller->AttachConn(conn.get())) {
        conn->watched_ = true;
        conns_[conn_name] = conn;
      }
    }
  }
}

// EPOLLIN handler
// this method is called in multiple thread;
// all we do in this function is to read fd and put data into conn->read_buf_
void Reactor::Reader(ReactorConn* conn, bool& closed) {
  int fd = conn->fd_;
  Buffer* rbuf = conn->read_buf_;
  if (rbuf == nullptr) {
    mtx_.lock();
    std::cerr << "[" << conn->GetName() << "] rbuf is nullptr, invalid status!!\n";
    mtx_.unlock();
    return;
  }
  char tmpbuf[kNetReadBufSize] = {0};
  // size_t n = FixedSizeReadToBuf(fd, tmpbuf, sizeof(tmpbuf));
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
    // TODO do more error checking
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
      conn->loop_->epoller->ModifyConn(conn);
    }
  }
}

// EPOLLOUT handler
// this method is called in multiple thread;
// all we do in this function is to write out all bytes in conn->write_buf_
void Reactor::Writer(ReactorConn* conn, bool& closed) {
  int fd = conn->fd_;
  Buffer* wbuf = conn->write_buf_;
  if (wbuf == nullptr) {
    mtx_.lock();
    std::cerr << "[" << conn->GetName() << "] wbuf is nullptr, invalid status!!\n";
    mtx_.unlock();
    return;
  }
  if (wbuf->Size() <= 0) {
    conn->SetMaskRead();
    conn->loop_->epoller->ModifyConn(conn);
    return;
  }
  size_t n = wbuf->Size();
  size_t nbytes =
      FixedSizeWriteFromBuf(fd, static_cast<const char*>(wbuf->BeginReadIndex()), n);
  if (nbytes > 0) {
    if (nbytes == n) {
      // all bytes have been sent
      conn->SetMaskRead();
      conn->loop_->epoller->ModifyConn(conn);
      wbuf->Reset();
      // TODO: is it reasonable to put write callback here?
      if (ev_write_handler_ != nullptr) {
        bool close_after = false;
        ev_write_handler_(conn, close_after);
        if (close_after) {
          CloseConn(conn);
        }
      }
    } else {
      wbuf->ReaderIdxForward(nbytes);
    }
  }
  if (nbytes == 0 && wbuf->Size() != 0) {
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
  std::cout << "Assigned to eventloop-" << idx << '\n';
  return eventloops_[idx].get();
}

}  // namespace ahrimq