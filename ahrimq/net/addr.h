#ifndef _AHRIMQ_NET_ADDR_H_
#define _AHRIMQ_NET_ADDR_H_

#include <memory>
#include <string>

#include <arpa/inet.h>

namespace ahrimq {

/// @brief IPv4 address
class IPAddr4 {
 public:
  /// @brief Construct a new IPAddr4 object.
  /// @param ip string ipv4 address
  /// @param port port
  IPAddr4(const std::string &ip, uint16_t port);

  /// @brief Construct a new IPAddr4 object.
  /// @param addr ipv4 address in uint32 format
  /// @param port port
  IPAddr4(uint32_t addr = INADDR_ANY, uint16_t port = 0);

  /// @brief Return address in string format.
  /// @return 
  std::string ToString() const {
    return addr_str_ + ":" + std::to_string(port_);
  }

  /// @brief Get the port.
  /// @return 
  uint16_t GetPort() const {
    return port_;
  }

  /// @brief Get the struct sockaddr*.
  /// @return 
  sockaddr *GetAddr() const {
    return (struct sockaddr *)&addr_;
  }

  /// @brief Get the socklen.
  /// @return 
  socklen_t GetSockLen() const {
    return GetSockAddrLen();
  }

  /// @brief Get the sin_family.
  /// @return 
  int GetSinFamily() const {
    return addr_.sin_family;
  }

  /// @brief Get struct sockaddr_in&.
  /// @return 
  const struct sockaddr_in &GetSockAddrIn() const {
    return addr_;
  }

  /// @brief Get sock addr length.
  /// @return 
  socklen_t GetSockAddrLen() const {
    return sizeof(addr_);
  }

  /// @brief Sync address and port when inner addr_ field is set from the outside.
  void SyncPort();

 private:
  void InitAddrStr();

 private:
  std::string addr_str_ = "";
  uint16_t port_ = 0;
  struct sockaddr_in addr_;
};

typedef std::shared_ptr<IPAddr4> IPAddr4Ptr;

}  // namespace ahrimq

#endif  // _AHRIMQ_NET_ADDR_H_