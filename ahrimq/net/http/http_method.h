#ifndef _HTTP_METHOD_H_
#define _HTTP_METHOD_H_

#include <string>
#include <unordered_map>

namespace ahrimq {

namespace http {

// define HTTP request methods

constexpr static char* MethodGet = "GET";
constexpr static char* MethodHead = "HEAD";
constexpr static char* MethodPost = "POST";
constexpr static char* MethodPut = "PUT";
constexpr static char* MethodPatch = "PATCH";
constexpr static char* MethodDelete = "DELETE";
constexpr static char* MethodConnect = "CONNECT";
constexpr static char* MethodOptions = "OPTIONS";
constexpr static char* MethodTrace = "TRACE";

enum class Method { Get, Head, Post, Put, Patch, Delete, Connect, Options, Trace };

const static std::unordered_map<Method, std::string> MethodStringMapping = {
    {Get, MethodGet},         {Head, MethodHead},       {Post, MethodPost},
    {Put, MethodPut},         {Patch, MethodPatch},     {Delete, MethodDelete},
    {Connect, MethodConnect}, {Options, MethodOptions}, {Trace, MethodTrace}};

}  // namespace http

}  // namespace ahrimq

#endif  // _HTTP_METHOD_H_