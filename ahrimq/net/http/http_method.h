#ifndef _AHRIMQ_NET_HTTP_HTTP_METHOD_H_
#define _AHRIMQ_NET_HTTP_HTTP_METHOD_H_

#include <algorithm>
#include <string>
#include <unordered_map>

#include "base/str_utils.h"

namespace ahrimq {

namespace http {

// define HTTP request method strings
const static char* MethodGet = "GET";
const static char* MethodHead = "HEAD";
const static char* MethodPost = "POST";
const static char* MethodPut = "PUT";
const static char* MethodPatch = "PATCH";
const static char* MethodDelete = "DELETE";
const static char* MethodConnect = "CONNECT";
const static char* MethodOptions = "OPTIONS";
const static char* MethodTrace = "TRACE";

/// @brief Supported http method.
enum class HTTPMethod {
  Get,
  Head,
  Post,
  Put,
  Patch,
  Delete,
  Connect,
  Options,
  Trace
};

static std::unordered_map<HTTPMethod, std::string> httpMethodStringMapping{
    {HTTPMethod::Get, MethodGet},         {HTTPMethod::Head, MethodHead},
    {HTTPMethod::Post, MethodPost},       {HTTPMethod::Put, MethodPut},
    {HTTPMethod::Patch, MethodPatch},     {HTTPMethod::Delete, MethodDelete},
    {HTTPMethod::Connect, MethodConnect}, {HTTPMethod::Options, MethodOptions},
    {HTTPMethod::Trace, MethodTrace}};

static std::unordered_map<std::string, HTTPMethod> httpStringMethodMapping{
    {MethodGet, HTTPMethod::Get},         {MethodHead, HTTPMethod::Head},
    {MethodPost, HTTPMethod::Post},       {MethodPut, HTTPMethod::Put},
    {MethodPatch, HTTPMethod::Patch},     {MethodDelete, HTTPMethod::Delete},
    {MethodConnect, HTTPMethod::Connect}, {MethodOptions, HTTPMethod::Options},
    {MethodTrace, HTTPMethod::Trace}};

/// @brief Check if given http method string is supported.
/// @param method
/// @return
bool HTTPMethodSupported(const std::string& method);

}  // namespace http

}  // namespace ahrimq

#endif  // _AHRIMQ_NET_HTTP_HTTP_METHOD_H_