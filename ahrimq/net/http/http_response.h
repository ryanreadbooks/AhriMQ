#ifndef _AHRIMQ_NET_HTTP_HTTP_RESPONSE_H_
#define _AHRIMQ_NET_HTTP_HTTP_RESPONSE_H_

#include <list>
#include <memory>
#include "thirdparty/nlohmann/json.hpp"

#include "buffer/buffer.h"
#include "net/http/http_header.h"
#include "net/http/http_status.h"
#include "net/http/cookie.h"

namespace ahrimq {
namespace http {

/// @brief HTTPResponse represents a http response.
class HTTPResponse {
 public:
  explicit HTTPResponse(Buffer* wbuf);

  HTTPHeaderPtr GetHeader() const {
    return header_;
  }

  HTTPHeaderPtr& HeaderRef() {
    return header_;
  }

  /// @brief Add key-value into response header.
  /// @param key
  /// @param value
  void AddHeader(const std::string& key, const std::string& value);

  /// @brief Set key-value into response header.
  /// @param key
  /// @param value
  void SetHeader(const std::string& key, const std::string& value);

  /// @brief Check if header contains given key.
  /// @param key
  /// @return
  bool HeaderContains(const std::string& key);

  /// @brief Return response status code.
  /// @return
  int Status() const {
    return status_;
  }

  /// @brief Set the response status code.
  /// @param status
  void SetStatus(int status);

  /// @brief Reset the http response instance, including reset the status code and
  /// the response header.
  void Reset();

  /// @brief Organize response content into write buffer. Organize will organize
  /// response header and response body bytes into connection write buffer.
  /// @param wbuf
  void Organize(Buffer& wbuf) const;

  /// @brief Append char content to response write buffer. Constructing manually http
  /// format is needed when using this function.
  /// @param content
  void AppendConnBuffer(const std::string& content);

  /// @brief Append char content to response write buffer. Constructing manually http
  /// format is needed when using this function.
  /// @param content
  /// @param clen
  void AppendConnBuffer(const char* content, size_t clen);

  /// @brief Set response content-type.
  /// @param content_type
  void SetContentType(const std::string& content_type);

  /// @brief Set response character encoding.
  /// @param encoding
  void SetContentEncoding(const std::string& encoding);

  Buffer& UserBuffer() {
    return user_buf_;
  }

  /// @brief Convenient function to add plain text response body. Using this function
  /// to add response body is recommended. Note that this function will clear
  /// existing data in user buffer.
  /// @param text
  void MakeContentPlainText(const std::string& text);

  /// @brief Convenient function to add json content to response body. Using this
  /// function to add response body is recommended. Note that this function will
  /// clear existing data in user buffer.
  /// @param json
  void MakeContentJson(const std::string& json);

  void MakeContentJson(const nlohmann::json& json);

  /// @brief Convenient function to add html content to response body. Using this
  /// function to add response body is recommended when html content is short. Note
  /// that this function will clear existing data in user buffer.
  /// @param html
  void MakeContentSimpleHTML(const std::string& html);

  /// @brief Set response redirect to new url location
  /// @param url redirected url
  /// @param code redirected status code like 3xx
  void RedirectTo(const std::string& url, int code);

  void AddCookie(const Cookie& cookie);

  void AddCookie(Cookie&& cookie);

  // TODO implement and multipart response body

 private:
  // response status code
  int status_ = StatusBadRequest;
  // http response header
  HTTPHeaderPtr header_;
  // write buffer from connection to store write data
  Buffer* write_buf_;
  // buffer to store user data
  Buffer user_buf_;
  // cookies
  std::list<Cookie> cookies_;
};

typedef std::shared_ptr<HTTPResponse> HTTPResponsePtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _AHRIMQ_NET_HTTP_HTTP_RESPONSE_H_