#ifndef _HTTP_URL_H_
#define _HTTP_URL_H_

#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace ahrimq {
namespace http {

/// @brief URL is a wrapper for http reqeust-url
class URL {
 public:
  /// @brief Class Query represents extra parameters for query trailing at the
  /// request url. Those parameters are a list of key/value pairs separated with the
  /// '&' symbol. A URL instance contains a Query instance which is constructed when
  /// constructing or setting new string url.
  class Query {
    friend class URL;

   public:
    /// @brief Add value with given key.
    /// @param key
    /// @param value
    void Add(const std::string& key, const std::string& value);

    /// @brief  Set value with given key, original value will be overwritten.
    /// @param key
    /// @param value
    void Set(const std::string& key, const std::string& value);

    /// @brief Get the first value of given key, return "" if key not exists.
    /// @param key
    /// @return
    std::string Get(const std::string& key) const;

    /// @brief Get all values with given key.
    /// @param key
    /// @return
    std::vector<std::string> Values(const std::string& key) const;

    /// @brief Check given key exists.
    /// @param key
    /// @return
    bool Has(const std::string& key) const;

    /// @brief Delete given key.
    /// @param key
    void Del(const std::string& key);

    /// @brief Delete all key-values in query.
    void Clear();

    /// @brief Get the number of pairs of key-value.
    size_t Size() const {
      return params_.size();
    }

    bool Empty() const {
      return params_.empty();
    }

    void ParseString(const std::string& str);

    friend std::ostream& operator<<(std::ostream& os, const Query& query);

   private:
    std::unordered_map<std::string, std::vector<std::string>> params_;
  };

 public:
  URL(const std::string& url = "/");

  /// @brief Set new url string.
  /// @param s
  void Set(const std::string& s) {
    url_ = s;
    ParseQuery();
  }

  /// @brief Set new url string.
  /// @param s
  void Set(std::string&& s) {
    url_ = std::move(s);
    ParseQuery();
  }

  /// @brief Get url string.
  /// @return
  std::string String() const {
    return url_;
  }

  /// @brief Return the url string without query parameters.
  /// @return
  std::string StringWithQuery() const;

  /// @brief Reset url instance
  void Reset() {
    url_ = "";
    query_.Clear();
  }

  /// @brief Get query on url.
  /// @return
  const Query& GetQuery() const {
    return query_;
  }

  /// @brief Check if url has query parameters.
  /// @return
  bool HasQuery() const {
    return query_.Empty();
  }

  friend std::ostream& operator<<(std::ostream& os, const URL& url);

 private:
  /// @brief Parse url string and set query_ member.
  void ParseQuery();

 private:
  std::string url_;
  Query query_;
};

typedef std::shared_ptr<URL> URLPtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_URL_H_