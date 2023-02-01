#ifndef _HTTP_PAGES_H_
#define _HTTP_PAGES_H_

namespace ahrimq {
namespace http {

/// We define some default html pages here.

static const char* DEFAULT_400_PAGE =
    "<!DOCTYPE HTML>"
    "<title>400 Bad Request</title>"
    "<h1>Bad Request</h1>"
    "<p>The request can not be identified due to malformed request syntax, invalid "
    "request message framing, or deceptive request routing, etc.</p>";

static const char* DEFAULT_404_PAGE =
    "<!DOCTYPE HTML>"
    "<title>404 Not Found</title>"
    "<h1>Not Found</h1>"
    "<p>The requested URL was not found on the server.</p>";

static const char* DEFAULT_405_PAGE =
    "<!DOCTYPE HTML>"
    "<title>405 Method Not Allowed</title>"
    "<h1>Method Not Allowed</h1>"
    "<p>The method is not allowed for the requested URL.</p>";

static const char* DEFAULT_500_PAGE =
    "<!DOCTYPE HTML>"
    "<title>500 Internal Server Error</title>"
    "<h1>Internal Server Error</h1>"
    "<p>The server is not available now. Please try again later.</p>";

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_PAGES_H_