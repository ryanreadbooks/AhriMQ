#ifndef _HTTP_ROUTER_H_
#define _HTTP_ROUTER_H_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "net/http/http_request.h"
#include "net/http/http_response.h"

namespace ahrimq {
namespace http {

// typedef std::function<std::string(const HTTPRequest &, HTTPResponse &)>
// HTTPCallback;

/// Helper functionality
namespace detail {

/// @brief Return the longest comman prefix index between two strings. For example,
/// LongestCommonPrefix("abc", "ab") return 1; LongestCommonPrefix("abcde", "abcde")
/// returns 4; LongestCommonPrefix("abc", "abcdefwe") returns 2.
/// @param s1 the first string
/// @param s2 the second string
/// @return return -1 if no common prefix is found
int LongestCommonPrefix(const std::string &s1, const std::string &s2);

/// @brief Find the first "{xxx}" location in s and return the content wrapped inside
/// "{}". Note that string should contain slash(/) in order to use this.
/// @param s pattern string
/// @param outname output argument
/// @return return -1 if no "{}" found, return -2 if name invalid in s; return the
/// index of the first '{' if found and valid
int FindParamBracket(const std::string &s, std::string &outname);

/// @brief Check segment string is containing wildcard parameter, if true then return
/// its parameter name.
/// @param s segment string
/// @param outname parameter name
/// @return
int CheckWildcard(const std::string &s, std::string &outname);

/// @brief Find wildcard location.
/// @param s
/// @return
int FindWildcard(const std::string &s);

class RouteNode;
typedef std::shared_ptr<RouteNode> RouteNodePtr;

/// @brief Approximate radix tree node implementation to support http router
class RouteNode {
 public:
  /// @brief Params represents the parameters in url string
  class Params {
   public:
    std::vector<std::string> Keys() const;

    std::vector<std::string> Values() const;

    void Set(const std::string &key, const std::string &value);

    std::string Get(const std::string &key);

    void Reset();

   private:
    std::unordered_map<std::string, std::string> params_;
  };

  typedef std::function<std::string(const HTTPRequest &, HTTPResponse &,
                                    const Params &)>
      HTTPHandler;

 public:
  RouteNode();

  RouteNode(const std::string &segment);

  RouteNode(const std::string &segment, const std::string &wildcard_name);

  ~RouteNode();

  std::string Pattern() const {
    return pattern_;
  }

  std::string Segment() const {
    return segment_;
  }

  const HTTPHandler &Handler() const {
    return handler_;
  }

  bool InsertRoute(const std::string &url, const HTTPHandler &callback);

  RouteNode *SearchRoute(const std::string &url, Params &params);

  HTTPHandler SearchHandler(const std::string &url) const;

  /// @brief Check if node has wildcard parameter.
  /// @return
  bool IsWildNode() const {
    return !wildcard_name_.empty();
  }

 private:
  bool Insert(const std::string &url, const std::vector<std::string> &segments,
              size_t index, const HTTPHandler &callback);

  RouteNode *Search(const std::string &url, const std::vector<std::string> &segments,
                    size_t index, Params &params);

 private:
  // full pattern
  std::string pattern_;
  // pattern segment
  std::string segment_;
  // node children
  std::vector<RouteNodePtr> children_;
  // handler
  HTTPHandler handler_;
  // wildcard parameter name, if no wildcard parameter, it is ""
  std::string wildcard_name_;
};

}  // namespace detail

typedef detail::RouteNode::Params URLParams;
typedef detail::RouteNode::HTTPHandler HTTPCallback;

/// @brief An http router implementation.
/// Supported functionality: 1.
class HTTPRouter {
 public:
  /// @brief Construct an http router.
  HTTPRouter() = default;

  /// @brief Destroy the http router.
  ~HTTPRouter() = default;

  bool RegisterGet(HTTPCallback callback);

  bool RegisterPost(HTTPCallback callback);

 private:
  bool Register(HTTPMethod method, HTTPCallback &&callback);

 private:
  std::unordered_map<HTTPMethod, detail::RouteNode *> trees_;
  // TODO use a param pool
};

}  // namespace http
}  // namespace ahrimq

#endif  // _HTTP_ROUTER_H_