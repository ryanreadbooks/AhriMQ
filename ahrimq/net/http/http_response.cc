#include "net/http/http_response.h"

#include "base/str_utils.h"
#include "base/time_utils.h"

namespace ahrimq {
namespace http {

HTTPResponse::HTTPResponse(Buffer* wbuf)
    : header_(std::make_shared<HTTPHeader>()), write_buf_(wbuf) {
  // add some default header fields into response header
  header_->Add("Server", "AhriMQ/1.0");
}

HTTPResponse::~HTTPResponse() {
  user_buf_.Reset();
  write_buf_ = nullptr;
}

void HTTPResponse::AddHeader(const std::string& key, const std::string& value) {
  header_->Add(key, value);
}

void HTTPResponse::SetHeader(const std::string& key, const std::string& value) {
  header_->Set(key, value);
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
  write_buf_->Reset();
}

void HTTPResponse::Organize(Buffer& wbuf) const {
  const static char* colon_seperator = ": ";
  header_->Add("Date", time::GMTTimeNowString());  // response GMT time
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
      if (!StrCaseEqual(key, "date") && value.find(',') != std::string::npos) {
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
  // cookies
  for (const auto& cookie : cookies_) {
    wbuf.Append("Set-Cookie");
    wbuf.Append(colon_seperator, 2);
    cookie.Serialize(wbuf);
    wbuf.Append("\r\n");
  }
  wbuf.Append("\r\n");

  if (!user_buf_.Empty()) {
    // organize response body content from user_buf_
    wbuf.Append(user_buf_);
  }
}

void HTTPResponse::AppendConnBuffer(const std::string& content) {
  if (write_buf_ != nullptr) {
    write_buf_->Append(content);
  }
}

void HTTPResponse::AppendConnBuffer(const char* content, size_t clen) {
  if (write_buf_ != nullptr) {
    write_buf_->Append(content, clen);
  }
}

void HTTPResponse::SetContentType(const std::string& content_type) {
  header_->Set("Content-Type", content_type);
}

void HTTPResponse::SetContentEncoding(const std::string& encoding) {
  header_->Set("Content-Encoding", encoding);
}

void HTTPResponse::MakeContentPlainText(const std::string& text) {
  user_buf_.Reset();
  user_buf_.Append(text);
  header_->Set("Content-Type", "text/plain; charset=utf-8");
  header_->Set("Content-Length", std::to_string(text.size()));
}

void HTTPResponse::MakeContentJson(const std::string& json) {
  user_buf_.Reset();
  user_buf_.Append(json);
  header_->Set("Content-Type", "application/json; charset=utf-8");
  header_->Set("Content-Length", std::to_string(json.size()));
}

void HTTPResponse::MakeContentJson(const nlohmann::json& json) {
  user_buf_.Reset();
  // dump json instance to string
  user_buf_.Append(json.dump());
  header_->Set("Content-Type", "application/json; charset=utf-8");
  header_->Set("Content-Length", std::to_string(user_buf_.Size()));
}

void HTTPResponse::MakeContentSimpleHTML(const std::string& html) {
  user_buf_.Reset();
  user_buf_.Append(html);
  header_->Set("Content-Type", "text/html; charset=utf-8");
  header_->Set("Content-Length", std::to_string(html.size()));
}

void HTTPResponse::RedirectTo(const std::string& url, int code) {
  header_->Set("Location", url);
  SetStatus(code);
}

void HTTPResponse::AddCookie(const Cookie& cookie) {
  cookies_.emplace_back(cookie);
}

void HTTPResponse::AddCookie(Cookie&& cookie) {
  cookies_.push_back(cookie);
}

}  //  namespace http
}  // namespace ahrimq