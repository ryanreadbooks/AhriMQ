#ifndef _HTTP_STATUS_H_
#define _HTTP_STATUS_H_

#include <string>
#include <unordered_map>

namespace ahrimq {

namespace http {

// define some status code in HTTP

constexpr static int StatusContinue = 100;
constexpr static int StatusSwitchingProtocols = 101;

constexpr static int StatusOK = 200;
constexpr static int StatusCreated = 201;
constexpr static int StatusAccepted = 202;
constexpr static int StatusNonAuthoritativeInfo = 203;
constexpr static int StatusNoContent = 204;
constexpr static int StatusResetContent = 205;
constexpr static int StatusPartialContent = 206;

constexpr static int StatusMultipleChoices = 300;
constexpr static int StatusMovedPermanently = 301;
constexpr static int StatusFound = 302;
constexpr static int StatusSeeOther = 303;
constexpr static int StatusNotModified = 304;
constexpr static int StatusUseProxy = 305;
constexpr static int StatusTemporaryRedirect = 307;
constexpr static int StatusPermanentRedirect = 308;

constexpr static int StatusBadRequest = 400;
constexpr static int StatusUnauthorized = 401;
constexpr static int StatusPaymentRequired = 402;
constexpr static int StatusForbidden = 403;
constexpr static int StatusNotFound = 404;
constexpr static int StatusMethodNotAllowed = 405;
constexpr static int StatusNotAcceptable = 406;
constexpr static int StatusProxyAuthenticationRequired = 407;
constexpr static int StatusRequestTimeout = 408;
constexpr static int StatusConflict = 409;
constexpr static int StatusGone = 410;
constexpr static int StatusLengthRequired = 411;
constexpr static int StatusPreconditionFailed = 412;
constexpr static int StatusContentTooLarge = 413;
constexpr static int StatusRequestURITooLong = 414;
constexpr static int StatusUnsupportedMediaType = 415;
constexpr static int StatusRangeNotSatisfiable = 416;
constexpr static int StatusExpectationFailed = 417;

constexpr static int StatusInternalServerError = 500;
constexpr static int StatusNotImplemented = 501;
constexpr static int StatusBadGateway = 502;
constexpr static int StatusServiceUnavailable = 503;
constexpr static int StatusGatewayTimeout = 504;
constexpr static int StatusHTTPVersionNotSupported = 505;

const static std::unordered_map<int, std::string> StatusCodeStringMapping = {
    {StatusContinue, "Continue"},
    {StatusSwitchingProtocols, "Switching Protocols"},

    {StatusOK, "OK"},
    {StatusCreated, "Created"},
    {StatusAccepted, "Accepted"},
    {StatusNonAuthoritativeInfo, "Non-Authoritative Information"},
    {StatusNoContent, "No Content"},
    {StatusResetContent, "Reset Content"},
    {StatusPartialContent, "Partial Content"},

    {StatusMultipleChoices, "Multiple Choices"},
    {StatusMovedPermanently, "Moved Permanently"},
    {StatusFound, "Found"},
    {StatusSeeOther, "See Other"},
    {StatusNotModified, "Not Modified"},
    {StatusUseProxy, "Use Proxy"},
    {StatusTemporaryRedirect, "Temporary Redirect"},
    {StatusPermanentRedirect, "Permanent Redirect"},

    {StatusBadRequest, "Bad Request"},
    {StatusUnauthorized, "Unauthorized"},
    {StatusPaymentRequired, "Payment Required"},
    {StatusForbidden, "Forbidden"},
    {StatusNotFound, "Not Found"},
    {StatusMethodNotAllowed, "Method Not Allowed"},
    {StatusNotAcceptable, "Not Acceptable"},
    {StatusProxyAuthenticationRequired, "Proxy Authentication Required"},
    {StatusRequestTimeout, "Request Timeout"},
    {StatusConflict, "Conflict"},
    {StatusGone, "Gone"},
    {StatusLengthRequired, "Length Required"},
    {StatusPreconditionFailed, "Precondition Failed"},
    {StatusContentTooLarge, "Content Too Large"},
    {StatusRequestURITooLong, "URI Too Long"},
    {StatusUnsupportedMediaType, "Unsupported Media Type"},
    {StatusRangeNotSatisfiable, "Range Not Satisfiable"},
    {StatusExpectationFailed, "Expectation Failed"},

    {StatusInternalServerError, "Internal Server Error"},
    {StatusNotImplemented, "Not Implemented"},
    {StatusBadGateway, "Bad Gateway"},
    {StatusServiceUnavailable, "Service Unavailable"},
    {StatusGatewayTimeout, "Gateway Timeout"},
    {StatusHTTPVersionNotSupported, "HTTP Version Not Supported"}

};
}  // namespace http

}  // namespace ahrimq

#endif  // _HTTP_STATUS_H_