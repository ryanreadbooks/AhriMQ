#ifndef _AHRIMQ_NET_UTILS_H_
#define _AHRIMQ_NET_UTILS_H_

#include <cstdint>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>

namespace ahrimq {

constexpr static int OK = 0;
constexpr static int ERR = -1;

constexpr static int READ_EOF_REACHED = (1 << 1);
constexpr static int READ_SOCKET_CLOSED = (1 << 2);
constexpr static int READ_EOF_NOT_REACHED = (1 << 3);
constexpr static int READ_PROCESS_ERROR = (1 << 4);

size_t FixedSizeReadToBuf(int fd, char *buf, size_t len);

size_t FixedSizeReadToBuf(int fd, char *buf, size_t len, int *flag);

size_t FixedSizeWriteFromBuf(int fd, const char *buf, size_t len);

size_t SendFile(int infd, int outfd, size_t offset, size_t len);

int SetSocketOpts(int fd, int level, int optname, int val);

int SetFdNonBlock(int fd);

int SetTCPNoDelayOption(int fd, int val);

int EnableTCPNoDelay(int fd);

int DisableTCPNoDelay(int fd);

int SetReuseAddress(int fd);

int SetReusePort(int fd);

int SetTCPKeepAliveOption(int fd, int val);

int EnableTCPKeepAlive(int fd);

int DisableTCPKeepAlive(int fd);

int SetKeepAliveIdle(int fd, int idle);

int SetKeepAliveInterval(int fd, int interval);

int SetKeepAliveCnt(int fd, int cnt);

int SetKeepAliveConfig(int fd, int idle, int interval, int cnt);

static uint16_t HostToNet16(uint16_t host16) {
  return htobe16(host16);
}

static uint32_t HostToNet32(uint32_t host32) {
  return htobe32(host32);
}

static uint64_t HostToNet64(uint64_t host) {
  return htobe64(host);
}

static uint16_t NetToHost16(uint16_t net16) {
  return be16toh(net16);
}

static uint32_t NetToHost32(uint32_t net32) {
  return be32toh(net32);
}

static uint32_t NetToHost64(uint64_t net64) {
  return be64toh(net64);
}

}  // namespace ahrimq

#endif  // _AHRIMQ_NET_UTILS_H_