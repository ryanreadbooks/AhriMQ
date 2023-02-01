#ifndef _HTTP_RESPONSE_H_
#define _HTTP_RESPONSE_H_

#include <memory>

#include "buffer/buffer.h"
#include "net/http/http_header.h"
#include "net/http/http_status.h"

namespace ahrimq {
namespace http {

/// @brief HTTPResponse represents a http response.
class HTTPResponse {
 public:
  HTTPResponse(Buffer* wbuf);

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
  /// to add response body is recommended.
  /// @param text
  void MakeContentPlainText(const std::string& text);

  /// @brief Convenient function to add json content to response body. Using this
  /// function to add response body is recommended.
  /// @param json
  void MakeContentJson(const std::string& json);

  /// @brief Convenient function to add html content to response body. Using this
  /// function to add response body is recommended when html content is short.
  /// @param html
  void MakeContentSimpleHTML(const std::string& html);

  /// @brief Set response redirect.
  /// @param url redirected url
  /// @param code redirected status code like 3xx
  void Redirect(const std::string& url, int code);
  // TODO implement file and multipart response body

 private:
  // response status code
  int status_ = StatusBadRequest;
  // http response header
  HTTPHeaderPtr header_;
  // write buffer from connection to store write data
  Buffer* write_buf_;
  // buffer to store user data
  Buffer user_buf_;
  // TODO we need body pointer to point to the response body?
};

typedef std::shared_ptr<HTTPResponse> HTTPResponsePtr;

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_RESPONSE_H_