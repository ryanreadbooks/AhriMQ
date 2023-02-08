#ifndef _AHRIMQ_NET_HTTP_HTTP_HEADER_H_
#define _AHRIMQ_NET_HTTP_HTTP_HEADER_H_

#include <cstring>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

/// @brief HTTPHeader represents a http header
class HTTPHeader {
  struct CaseInsensitiveHasher {
    size_t operator()(const std::string& s) const {
      std::string n(s);
      StrInplaceToLower(n);
      return std::hash<std::string>()(n);
    }
  };

  struct CaseInsensitiveComparator {
    bool operator()(const std::string& s1, const std::string& s2) const {
      return strcasecmp(s1.c_str(), s2.c_str()) == 0;
    }
  };

 public:
  // clang-format off
  using MemberMapType = std::unordered_map<std::string, 
                      std::vector<std::string>, 
                      CaseInsensitiveHasher,
                      CaseInsensitiveComparator>;
  // clang-format on

  HTTPHeader() = default;

  HTTPHeader(const HTTPHeader&) = default;

  HTTPHeader& operator=(const HTTPHeader&) = default;

  HTTPHeader(HTTPHeader&& other);

  /// @brief Add field into http header.
  /// @param key
  /// @param value
  void Add(const std::string& key, const std::string& value);

  /// @brief Delete field from http header.
  /// @param key
  void Del(const std::string& key);

  /// @brief Gets the first value associated with the given key.
  /// @param key
  /// @return
  std::string Get(const std::string& key) const;

  /// @brief Gets all values associated with the given key.
  /// @param key
  /// @return
  std::vector<std::string> Values(const std::string& key) const;

  /// @brief Set value with given key. If key already exists, the original value will
  /// be overwritten.
  /// @param key
  /// @param value
  void Set(const std::string& key, const std::string& value);

  /// @brief Get all field keys.
  /// @return
  std::vector<std::string> AllFieldKeys() const;

  /// @brief Get all field values.
  /// @return
  std::vector<std::vector<std::string>> AllFieldValues() const;

  /// @brief Clear all header fields.
  void Clear();

  /// @brief Check given field exists.
  /// @param key
  /// @return
  bool Has(const std::string& key) const;

  /// @brief Compares the first value with the given key is equal to target.
  /// @param key given key
  /// @param target target value
  /// @return
  bool Equals(const std::string& key, const std::string& target) const;

  /// @brief Compares the first value with the given key is equal to target.
  /// @param key given key
  /// @param target target value
  /// @return
  bool Equals(const std::string& key, const char* target) const;

  bool CaseEquals(const std::string& key, const std::string& target) const;

  bool CaseEquals(const std::string& key, const char* target) const;

  bool Contains(const std::string& key, const std::string& target) const;

  /// @brief Return the underneath std::unordered_map.
  /// @return
  const MemberMapType& Members() const {
    return members_;
  }

  size_t Size() const {
    return members_.size();
  }

 private:
  MemberMapType members_;
};

typedef std::shared_ptr<HTTPHeader> HTTPHeaderPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _AHRIMQ_NET_HTTP_HTTP_HEADER_H_