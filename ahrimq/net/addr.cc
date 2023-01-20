#include "addr.h"
#include "utils.h"

namespace ahrimq {

IPAddr4::IPAddr4(const std::string &ip, uint16_t port) : addr_str_(ip), port_(port) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = HostToNet16(port_);
  inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr.s_addr);
  InitAddrStr();
}

IPAddr4::IPAddr4(uint32_t addr, uint16_t port) : port_(port) {
  memset(&addr_, 0, sizeof(addr_));
  addr_.sin_family = AF_INET;
  addr_.sin_port = HostToNet16(port_);
  InitAddrStr();
}

void IPAddr4::SyncPort() {
  port_ = NetToHost16(addr_.sin_port);
  InitAddrStr();
}

void IPAddr4::InitAddrStr() {
  char buf[INET_ADDRSTRLEN + 1];
  memset(buf, 0, sizeof(buf));
  inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
  addr_str_ = buf;
}

}  // namespace ahrimq