#ifndef _HTTP_HEADER_H_
#define _HTTP_HEADER_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ahrimq {
namespace http {

/// @brief HTTPHeader represents a http header
class HTTPHeader {
 public:
  HTTPHeader() = default;

  HTTPHeader(const HTTPHeader&) = default;

  HTTPHeader& operator=(const HTTPHeader&) = default;

  HTTPHeader(HTTPHeader&& other);

  /// @brief Add field into http header.
  /// @param key
  /// @param value
  void Add(std::string key, std::string value);

  /// @brief Delete field from http header.
  /// @param key
  void Del(const std::string& key);

  /// @brief Gets the first value associated with the given key.
  /// @param key
  /// @return
  std::string Get(const std::string& key);

  /// @brief Gets all values associated with the given key.
  /// @param key
  /// @return
  std::vector<std::string> Values(const std::string& key);

  /// @brief Set value with given key. If key already exists, the original value will
  /// be overwritten.
  /// @param key
  /// @param value
  void Set(const std::string& key, std::string value);

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
  bool Equals(const std::string& key, const std::string& target);

  /// @brief Compares the first value with the given key is equal to target.
  /// @param key given key
  /// @param target target value
  /// @return
  bool Equals(const std::string& key, const char* target);

  /// @brief Return the underneath std::unordered_map.
  /// @return
  const std::unordered_map<std::string, std::vector<std::string>>& Members() const {
    return members_;
  }

 private:
  // FIXME: key is case-insensative
  std::unordered_map<std::string, std::vector<std::string>> members_;
};

typedef std::shared_ptr<HTTPHeader> HTTPHeaderPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_HEADER_H_