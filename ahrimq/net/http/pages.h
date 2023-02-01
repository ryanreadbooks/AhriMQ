#ifndef _HTTP_PAGES_H_
#define _HTTP_PAGES_H_

namespace ahrimq {
namespace http {

/// We define some default html pages here.
static const char* DEFAULT_400_HTML = "400.html";
static const char* DEFAULT_400_PAGE =
    "<!DOCTYPE HTML>"
    "<title>400 Bad Request</title>"
    "<h1>Bad Request</h1>"
    "<p>The request can not be identified due to malformed request syntax, invalid "
    "request message framing, or deceptive request routing, etc.</p>";

static const char* DEFAULT_404_HTML = "404.html";
static const char* DEFAULT_404_PAGE =
    "<!DOCTYPE HTML>"
    "<title>404 Not Found</title>"
    "<h1>Not Found</h1>"
    "<p>The requested URL was not found on the server.</p>";

static const char* DEFAULT_405_HTML = "405.html";
static const char* DEFAULT_405_PAGE =
    "<!DOCTYPE HTML>"
    "<title>405 Method Not Allowed</title>"
    "<h1>Method Not Allowed</h1>"
    "<p>The method is not allowed for the requested URL.</p>";

static const char* DEFAULT_413_HTML = "413.html";
static const char* DEFAULT_413_PAGE =
    "<!DOCTYPE HTML>"
    "<title>413 Content Too Large</title>"
    "<h1>Content Too Large</h1>"
    "<p>The request body content is too large.</p>";

static const char* DEFAULT_500_HTML = "500.html";
static const char* DEFAULT_500_PAGE =
    "<!DOCTYPE HTML>"
    "<title>500 Internal Server Error</title>"
    "<h1>Internal Server Error</h1>"
    "<p>The server is not available now. Please try again later.</p>";

static const char* DEFAULT_501_HTML = "501.html";
static const char* DEFAULT_501_PAGE =
    "<!DOCTYPE HTML>"
    "<title>501 Not Implemented</title>"
    "<h1>Not Implemented</h1>"
    "<p>Method not implemented</p>";

static const char* DEFAULT_ERR_HTML = "error.html";
static const char* DEFAULT_ERR_PAGE =
    "<!DOCTYPE HTML>"
    "<title>Error Occurs</title>"
    "<h1>Error Occurs</h1>"
    "<p>Unknown error</p>";

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_PAGES_H_