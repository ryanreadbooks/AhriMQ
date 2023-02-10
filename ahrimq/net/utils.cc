#include "utils.h"

#include <sys/ioctl.h>

namespace ahrimq {

size_t FixedSizeReadToBuf(int fd, char *buf, size_t len) {
  size_t total_read = 0;
  ssize_t bytes_read = 0;
  memset(buf, 0, len);
  while (total_read < len) {
    bytes_read = read(fd, buf + total_read, len - total_read);
    if (bytes_read > 0) {  // read normally
      total_read += bytes_read;
    } else if (bytes_read == 0) {  // remote peer closed
      break;
    } else if (bytes_read == -1) {  // error
      if (errno == EINTR) {
        continue;
      } else {
        // errno == EAGAIN or errno == EWOULDBLOCK means can read no more
        break;
      }
    }
  }
  return total_read;
}

size_t FixedSizeReadToBuf(int fd, char *buf, size_t len, int *flag) {
  *flag = READ_EOF_NOT_REACHED;
  size_t total_read = 0;
  ssize_t bytes_read = 0;
  memset(buf, 0, len);
  while (total_read < len) {
    size_t n_will_read = len - total_read;
    bytes_read = read(fd, buf + total_read, n_will_read);
    if (bytes_read > 0) {
      total_read += bytes_read;
    } else if (bytes_read == 0) {
      *flag = READ_SOCKET_CLOSED;
      break;
    } else if (bytes_read == -1) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        *flag = READ_EOF_REACHED;
        break;
      } else {
        *flag = READ_PROCESS_ERROR;
        break;
      }
    }
  }
  size_t remain = 0;
  // try to retrieve the remaining bytes in socket read buffer
  ioctl(fd, FIONREAD, &remain);
  if (remain == 0 && *flag == READ_EOF_NOT_REACHED) {
    *flag = READ_EOF_REACHED;
  }
  return total_read;
}

size_t ReadToBuffer(int fd, Buffer &buffer, int *flag) {
  *flag = READ_EOF_NOT_REACHED;
  size_t total_read = 0;
  ssize_t bytes_read = 0;
  // we need manual controlling of Buffer index
  static size_t kFixedSize = 4096;
  while (true) {
    size_t available = buffer.WritableBytes();
    if (available < kFixedSize) {
      buffer.EnsureBytesForWrite(kFixedSize);
    }
    // try to read kFixedSize bytes every time
    bytes_read = read(fd, buffer.BeginWritePointer(), kFixedSize);
    if (bytes_read > 0) {
      total_read += bytes_read;
      buffer.WriterIdxForward(bytes_read);
    } else if (bytes_read == 0) {
      *flag = READ_SOCKET_CLOSED;
      break;
    } else if (bytes_read == -1) {
      if (errno == EINTR) {
        continue;
      } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        *flag = READ_EOF_REACHED;
        break;
      } else {
        *flag = READ_PROCESS_ERROR;
        printf("ReadToBuffer error at fd %d, %s\n", fd, strerror(errno));
        break;
      }
    }
  }
  size_t remain = 0;
  // try to retrieve the remaining bytes in socket read buffer
  ioctl(fd, FIONREAD, &remain);
  if (remain == 0 && *flag == READ_EOF_NOT_REACHED) {
    *flag = READ_EOF_REACHED;
  }
  return total_read;
}

size_t FixedSizeWriteFromBuf(int fd, const char *buf, size_t len) {
  size_t total_written = 0;
  ssize_t bytes_written = 0;
  while (total_written < len) {
    bytes_written = send(fd, buf + total_written, len - total_written, 0);
    if (bytes_written > 0) {
      total_written += bytes_written;
    } else if (bytes_written == -1) {  // error
      if (errno == EINTR) {
        continue;
      } else {
        break;
      }
    }
  }
  return total_written;
}

size_t SendFile(int infd, int outfd, size_t offset, size_t len) {
  // we should make sure all bytes from infd is sent to outfd, unless error occurs
  size_t total_sent = 0;
  ssize_t bytes_sent = 0;
  off_t cur = offset;
  while (total_sent < len) {
    size_t to_send = len - total_sent;
    bytes_sent = sendfile(outfd, infd, &cur, to_send);
    if (bytes_sent > 0) {
      // sent ok
      // cur is set to the offset of the byte following the last byte that was read
      total_sent += bytes_sent;
    } else if (bytes_sent == -1) {
      // error occurs
      if (errno == EINTR) {
        continue;
      } else {
        break;
      }
    } else {
      // bytes_sent == 0
      break;
    }
  }
  return total_sent;
}

int SetSocketOpts(int fd, int level, int optname, int val) {
  return setsockopt(fd, level, optname, &val, sizeof(val)) == -1 ? ERR : OK;
}

int SetFdNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags == -1) {
    std::cerr << "fcntl(F_GETFL): " << strerror(errno) << std::endl;
    return ERR;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    std::cerr << "fcntl(F_SETFL, O_NONBLOCK): " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int SetTCPNoDelayOption(int fd, int val) {
  if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt TCP_NODELAY: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int EnableTCPNoDelay(int fd) {
  return SetTCPNoDelayOption(fd, 1);
}

int DisableTCPNoDelay(int fd) {
  return SetTCPNoDelayOption(fd, 0);
}

int SetReuseAddress(int fd) {
  int val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt SO_REUSEADDR: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int SetReusePort(int fd) {
  int val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt SO_REUSEPORT: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int SetTCPKeepAliveOption(int fd, int val) {
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt SO_KEEPALIVE: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int EnableTCPKeepAlive(int fd) {
  return SetTCPKeepAliveOption(fd, 1);
}

int DisableTCPKeepAlive(int fd) {
  return SetTCPKeepAliveOption(fd, 0);
}

int SetKeepAliveIdle(int fd, int idle) {
  int val = idle;
  if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt TCP_KEEPIDLE: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int SetKeepAliveInterval(int fd, int interval) {
  int val = interval;
  if (setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt TCP_KEEPINTVL: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int SetKeepAliveCnt(int fd, int cnt) {
  int val = cnt;
  if (setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt TCP_KEEPCNT: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

int SetKeepAliveConfig(int fd, int idle, int interval, int cnt) {
  int val = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt SO_KEEPALIVE: " << strerror(errno) << std::endl;
    return ERR;
  }
  val = idle;
  if (setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt TCP_KEEPIDLE: " << strerror(errno) << std::endl;
    return ERR;
  }
  val = interval;
  if (setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt TCP_KEEPINTVL: " << strerror(errno) << std::endl;
    return ERR;
  }
  val = cnt;
  if (setsockopt(fd, SOL_TCP, TCP_KEEPCNT, &val, sizeof(val)) == -1) {
    std::cerr << "setsockopt TCP_KEEPCNT: " << strerror(errno) << std::endl;
    return ERR;
  }
  return OK;
}

}  // namespace ahrimq