#ifndef _NET_UTILS_H_
#define _NET_UTILS_H_

#include <cstdint>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace ahrimq {

constexpr static int OK = 0;
constexpr static int ERR = -1;

size_t ReadToBuf(int fd, char *buf, size_t len);

size_t WriteFromBuf(int fd, const char *buf, size_t len);

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

#endif  // _NET_UTILS_H_