#include "net/http/http_response.h"

#include "base/time_utils.h"

namespace ahrimq {
namespace http {

HTTPResponse::HTTPResponse(Buffer* wbuf)
    : header_(std::make_shared<HTTPHeader>()), write_buf_(wbuf) {
  // add some default header fields into response header
  header_->Add("Server", "AhriMQ/1.0");
}

void HTTPResponse::AddHeader(std::string key, std::string value) {
  header_->Add(std::move(key), std::move(key));
}

void HTTPResponse::SetHeader(std::string key, std::string value) {
  header_->Set(std::move(key), std::move(key));
}

bool HTTPResponse::HeaderContains(const std::string& key) {
  return header_->Has(key);
}

void HTTPResponse::SetStatus(int status) {
  if (StatusCodeStringMapping.count(status) != 0) {
    // status supported
    status_ = status;
  }
}

void HTTPResponse::Reset() {
  header_->Clear();
  status_ = StatusBadRequest;
  header_->Add("Server", "AhriMQ/1.0");
  write_buf_ = nullptr;
}

void HTTPResponse::Organize(Buffer& wbuf) const {
  const static char* colon_seperator = ": ";
  header_->Add("Date", GMTTimeNow());  // response GMT time
  // response line
  char buf[64] = {0};
  std::sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status_,
               StatusCodeStringMapping[status_].c_str());
  wbuf.Append(buf, std::strlen(buf));

  // organize response header
  for (const auto& item : header_->Members()) {
    const std::string& key = item.first;
    const std::vector<std::string>& values = item.second;
    if (values.empty()) {
      continue;
    }
    wbuf.Append(key);
    wbuf.Append(colon_seperator, 2);
    for (size_t i = 0; i < values.size(); i++) {
      const std::string& value = values[i];
      if (value.find(',') != std::string::npos) {
        wbuf.Append("\"");
        wbuf.Append(value);
        wbuf.Append("\"");
      } else {
        wbuf.Append(value);
      }
      // every elements are seperated by comma
      if (i != values.size() - 1) {
        wbuf.Append(",");
      }
    }
    wbuf.Append("\r\n");
  }
  wbuf.Append("\r\n");
  // organize response body
  // TODO, may be we should send request body by another way, because response body
  // may contain file data and other stuff.
}

void HTTPResponse::AppendBuffer(const std::string& content) {
  if (write_buf_ != nullptr) {
    write_buf_->Append(content);
  }
}

void HTTPResponse::AppendBuffer(const char* content, size_t clen) {
  if (write_buf_ != nullptr) {
    write_buf_->Append(content, clen);
  }
}

void HTTPResponse::SetContentType(const std::string& content_type) {
  header_->Set("Content-Type", content_type);
}

}  //  namespace http
}  // namespace ahrimq