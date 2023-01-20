#include "reactor.h"

using namespace std::placeholders;  // _1, _2

namespace ahrimq {

IOReadWriteStatus ReadFromFdToBuf(int fd, Buffer& buf) {
  IOReadWriteStatus status{.n_bytes = 0, .statuscode = kStatusNormal};
  if (fd == -1) {
    status.n_bytes = 0;
    status.statuscode = kStatusError;
    return status;
  }
  size_t total_bytes = 0;
  char tmpbuf[kNetReadBufSize] = {0};
  while (true) {
    ssize_t nbytes = read(fd, tmpbuf, sizeof(tmpbuf));
    if (nbytes == 0) {
      /* remote closed */
      status.statuscode = kStatusClosed;
      break;
    } else if (nbytes > 0) {
      total_bytes += nbytes;
      buf.Append(tmpbuf, nbytes);
    } else {  // nbytes == -1
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        status.statuscode = kStatusExhausted;
        break;
      } else if (errno == EINTR) {
        continue;
      } else {
        status.statuscode = kStatusError;
        break;
      }
    }
  }
  status.n_bytes = total_bytes;
  return status;
}

uint64_t Reactor::next_conn_id_ = 1;

Reactor::Reactor(const std::string& ip, uint16_t port, uint32_t num)
    : num_loop_(num), rand_engine_(std::time(nullptr)) {
  if (num == 0) {
    num_loop_ = std::thread::hardware_concurrency();
  }
  /* init eventloop */
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
  /* destroy all conn */
  eventloops_[0]->epoller->DetachConn(acceptor_.get());
  acceptor_.reset();
  for (auto&& loop : eventloops_) {
    loop->Stop();
  }
}

void Reactor::React() {
  /* start background threads and start running */
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
  /* bind acceptor fd to addr */
  if (bind(lfd, addr_->GetAddr(), addr_->GetSockAddrLen()) == -1) {
    std::cerr << "bind acceptor address failed : " << strerror(errno) << std::endl;
    return false;
  }
  acceptor_ = std::make_shared<TCPConn>(
      lfd, EPOLLIN, std::bind(&Reactor::Acceptor, this, _1, _2), nullptr,
      eventloops_[0].get(), "conn-acceptor");

  listen(lfd, SOMAXCONN);
  eventloops_[0]->epoller->AttachConn(acceptor_.get());
  acceptor_->watched_ = true;
  return true;
}

void Reactor::Acceptor(TCPConn* conn, bool& closed) {
  /* accept incoming connection(session) and process */
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
    TCPConnPtr conn = std::make_shared<TCPConn>(
        remote_fd, EPOLLIN, std::bind(&Reactor::Reader, this, _1, _2),
        std::bind(&Reactor::Writer, this, _1, _2), selected_loop, conn_name);
    if (conn != nullptr) {
      /* attach new session into epoll */
      SetFdNonBlock(remote_fd);
      conn->SetNoDelay(conn_nodelay_);
      /* we use tcp keepalive to close broken socket connection */
      conn->SetKeepAlive(conn_keepalive_);
      conn->SetKeepAlivePeriod(conn_keepalive_interval_);
      conn->SetKeepAliveCount(conn_keepalive_cnt_);
      if (selected_loop->epoller->AttachConn(conn.get())) {
        conn->watched_ = true;
        conns_[conn_name] = conn;
      }
    }
  }
}

// EPOLLIN handler
// this method is called in multiple thread;
void Reactor::Reader(TCPConn* conn, bool& closed) {
  int fd = conn->fd_;
  Buffer& rbuf = conn->read_buf_;
  IOReadWriteStatus status = ReadFromFdToBuf(fd, rbuf);
  size_t nbytes = status.n_bytes;
  ReadWriteStatusCode stcode = status.statuscode;
  if ((nbytes == 0 && stcode == kStatusClosed) || stcode == kStatusError) {
    /* conn closed */
    conn->watched_ = false;
    std::string cachename = conn->name_;
    close(fd);
    rbuf.Reset();
    conns_.erase(conn->name_);
    closed = true;
    std::cout << "tcp connection: " << cachename << " closed\n";
    return;
  }
  if (stcode == kStatusExhausted) {
    /* all data in socket read buffer has been read, now we call the callback */
    if (on_read_done_cb_ != nullptr) {
      on_read_done_cb_(conn);
    }
    if (conn->write_buf_.ReadableBytes() > 0) {
      conn->SetMaskWrite();
      conn->loop_->epoller->ModifyConn(conn);
    }
  }
}

// EPOLLOUT handler
// this method is called in multiple thread;
void Reactor::Writer(TCPConn* conn, bool& closed) {
  int fd = conn->fd_;
  Buffer& wbuf = conn->write_buf_;
  if (wbuf.ReadableBytes() <= 0) {
    conn->SetMaskRead();
    conn->loop_->epoller->ModifyConn(conn);
    return;
  }
  size_t n = wbuf.ReadableBytes();
  size_t nbytes = WriteFromBuf(fd, static_cast<const char*>(wbuf.BeginRead()), n);
  if (nbytes > 0) {
    if (nbytes == n) {
      /* all bytes have been sent */
      conn->SetMaskRead();
      conn->loop_->epoller->ModifyConn(conn);
      wbuf.Reset();
    } else {
      wbuf.ReaderIdxForward(nbytes);
    }
  }
  if (nbytes == 0 && wbuf.ReadableBytes() != 0) {
    conn->watched_ = false;
    wbuf.Reset();
    close(fd);
    conns_.erase(conn->name_);
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