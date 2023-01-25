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

  /// @brief add field into http header
  /// @param key
  /// @param value
  void Add(std::string key, std::string value);

  /// @brief delete field from http header
  /// @param key
  void Del(const std::string& key);

  /// @brief gets the first value associated with the given key
  /// @param key
  /// @return
  std::string Get(const std::string& key);

  /// @brief gets all values associated with the given key
  /// @param key 
  /// @return  
  std::vector<std::string> Values(const std::string& key);

  /// @brief
  /// @param key
  /// @param value
  void Set(const std::string& key, std::string value);

  /// @brief get all field keys
  /// @return
  std::vector<std::string> AllFieldKeys() const;

  /// @brief get all field values
  /// @return
  std::vector<std::vector<std::string>> AllFieldValues() const;

  /// @brief clear all header fields
  void Clear();

  /// @brief check given field exists
  /// @param key
  /// @return
  bool Has(const std::string& key) const;

  const std::unordered_map<std::string, std::vector<std::string>>& Members() const {
    return members_;
  }

 private:
  // FIXME: valute type should vector<string>
  // FIXME: key is case-insensative
  std::unordered_map<std::string, std::vector<std::string>> members_;
};

typedef std::shared_ptr<HTTPHeader> HTTPHeaderPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_HEADER_H_