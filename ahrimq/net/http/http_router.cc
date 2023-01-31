#include "net/http/http_router.h"

#include <iostream>

#include "base/str_utils.h"

namespace ahrimq {
namespace http {

namespace detail {

int LongestCommonPrefix(const std::string& s1, const std::string& s2) {
  int i = 0;
  int limit = std::min(s1.size(), s2.size());
  while (i < limit && s1[i] == s2[i]) {
    i++;
  }
  return i - 1;
}

int FindParamBracket(const std::string& s, std::string& out_name) {
  int idx;
  bool closed = false;
  int right_idx = 0;
  for (idx = 0; idx < int(s.size()); idx++) {
    if (s[idx] != '{') {
      continue;
    }
    // try to find '}'
    for (int j = idx + 1; j < int(s.size()); j++) {
      if (s[j] == '{') {
        // pattern like '/a/b/{sd{}/xxx' is invalid
        return -2;  // invalid
      } else if (s[j] == '}') {
        if (closed) {
          // pattern like /a/b/{abd}}/xxx is invalid
          return -2;
        }
        closed = true;
        right_idx = j;
      } else if (s[j] == '/') {
        if (s[j - 1] != '}') {
          // pattern like /a/b/{abs}xxx/sdsd is invalid
          // {xx} can only appears as suffix
          return -2;
        }
        // we treat '/' as the end of a segment, we can return now
        if (closed) {
          std::string substr = s.substr(idx + 1, right_idx - idx - 1);
          if (substr.empty()) {
            return -2;
          }
          out_name.swap(substr);
          return idx;
        }
        break;  // not found
      }
    }
  }
  // not found
  return -1;
}

int CheckWildcard(const std::string& s, std::string& outname) {
  auto begin = s.begin();
  auto end = s.end();
  auto idx = std::find(begin, end, '{');
  if (idx == end) {
    return -1;  // not found
  }
  // found '{'
  // check for invalid situation
  auto idx2 = std::find(idx + 1, end, '}');
  if (idx2 == end) {
    // '}' not found
    return -2;
  }
  // '}' found
  if (idx2 + 1 != end) {
    return -2;  // '}' is not the last character
  }
  std::string tmp = std::string(idx + 1, idx2);
  if (tmp.empty()) {
    // empty wildcard name is not allowed
    return -2;
  }
  outname.swap(tmp);
  return std::distance(begin, idx);
}

std::vector<std::string> RouteNode::Params::Keys() const {
  std::vector<std::string> keys;
  keys.reserve(params_.size());
  for (const auto& item : params_) {
    keys.emplace_back(item.first);
  }
  return keys;
}

std::vector<std::string> RouteNode::Params::Values() const {
  std::vector<std::string> values;
  values.reserve(params_.size());
  for (const auto& item : params_) {
    values.emplace_back(item.second);
  }
  return values;
}

void RouteNode::Params::Set(const std::string& key, const std::string& value) {
  params_[key] = value;
}

std::string RouteNode::Params::Get(const std::string& key) {
  return params_[key];
}

void RouteNode::Params::Reset() {
  params_.clear();
}

RouteNode::RouteNode() {}

RouteNode::RouteNode(const std::string& segment)
    : segment_(std::move(segment)), handler_(nullptr) {}

RouteNode::RouteNode(const std::string& segment, const std::string& wildcard_name)
    : segment_(segment), handler_(nullptr), wildcard_name_(wildcard_name) {}

RouteNode::~RouteNode() {}

bool RouteNode::InsertRoute(const std::string& url, const HTTPCallback& callback) {
  if (url == "/") {
    // TODO special handling when url == "/"
    if (!pattern_.empty()) {
      return false;
    }
    pattern_ = "/";
    segment_ = "/";
    handler_ = callback;
    return true;
  }
  std::vector<std::string> segments;
  StrSplit(url, '/', segments);
  if (segments.empty()) {
    return false;
  }
  return Insert(url, segments, 0, callback);
}

RouteNode* RouteNode::SearchRoute(const std::string& url, Params& params) {
  if (url == pattern_) {
    return this;
  }
  std::vector<std::string> segments;
  StrSplit(url, '/', segments);
  if (segments.empty()) {
    return nullptr;
  }
  return Search(url, segments, 0, params);
}

bool RouteNode::Insert(const std::string& url,
                       const std::vector<std::string>& segments, size_t index,
                       const HTTPCallback& callback) {
  if (index == segments.size()) {
    // reach destination node
    if (pattern_.empty()) {
      pattern_ = url;
      handler_ = callback;
      return true;
    }
    // url already exists, insertion failed
    return false;
  }
  // try to find segments[index] in children nodes
  RouteNodePtr next = nullptr;
  std::string child_wildname = "";
  int wildcard_valid = CheckWildcard(segments[index], child_wildname);
  if (wildcard_valid == -2) {
    // invalid
    return false;
  }
  for (auto& child : children_) {  // lookup in every child
    if (!child->IsWildNode()) {    // child has no wildcard
      if (wildcard_valid == -1) {  // no wildcard found in segment
        if (child->segment_ == segments[index]) {
          next = child;
          break;
        }
      } else {                            // wildcard found in segment
        if (segments[index][0] == '{') {  // wildcard in segment pattern like "{yyy}"
          std::cerr << segments[index] << " is in conflict with " << child->segment_
                    << std::endl;
          return false;
        } else {  // wildcard in segment pattern like yyy"{yyy}"
          // we need to check the prefix "yyy" to decide it is valid or not
          if (StrEqual(child->segment_, segments[index], wildcard_valid)) {
            // prefix of wildcard is the same as child segment
            // insertion is not allowed
            std::cerr << segments[index] << " is in conflict with "
                      << child->segment_ << std::endl;
            return false;
          }
        }
      }
    } else {                            // child has wildcard
      if (child->segment_[0] == '{') {  // child wildcard is like "{xxx}"
        // if child has wildcard, then no extra route is allowed to get through this
        // node unless they have the exact same wildcard pattern
        if (child->segment_ == segments[index]) {
          next = child;
          break;
        }
        std::cerr << segments[index] << " is in conflict with " << child->segment_
                  << std::endl;
        return false;
      } else {                       // child wildcard is like "xxx{xxx}"
        if (wildcard_valid == -1) {  // no wildcard in segment
          size_t child_wild_loc = child->segment_.find('{');
          if (StrEqual(segments[index], child->segment_, child_wild_loc) &&
              segments[index].size() > child_wild_loc) {
            return false;
          }
        } else {                            // has wildcard in segment
          if (segments[index][0] == '{') {  // wildcard is like "{yyy}""
            return false;
          }
          // wildcard is like "yyy{yyy}"
          // check longes common prefix
          int cpfx = LongestCommonPrefix(child->segment_, segments[index]);
          int l1 = child->segment_.size() - 1;
          int l2 = segments[index].size() - 1;
          if (cpfx != -1 && (cpfx == l1 || cpfx == l2)) {
            return false;
          }
        }
      }
    }
  }

  // can not found child, we need to create a new child
  if (next == nullptr) {
    children_.emplace_back(
        std::make_shared<RouteNode>(segments[index], child_wildname));
    next = children_.back();
  }
  return next->Insert(url, segments, index + 1, callback);
}

RouteNode* RouteNode::Search(const std::string& url,
                             const std::vector<std::string>& segments, size_t index,
                             Params& params) {
  if (index == segments.size()) {
    if (wildcard_name_.empty()) {
      if (segments[index - 1] == segment_) {
        // found (no wildcard)
        return this;
      }
    } else {
      // check if wildcard match segments[index - 1]
      if (segment_[0] == '{') {
        return this;
      } else {
        // check prefix match exactly
        return this;
      }
    }
  }
  // do not match in this node, we need to find its children
  RouteNode* found = nullptr;
  for (auto& child : children_) {
    if (!child->IsWildNode()) {
      if (child->segment_ == segments[index]) {
        found = child.get();
        break;
      }
    } else {
      // child contains wildcard parameter
      // vague matching
      if (child->segment_[0] == '{') {
        // child segment is like "{xxx}"
        // universal matching
        found = child.get();
        params.Set(child->wildcard_name_, segments[index]);
        break;
      } else {
        // child segment is like "abc{xxx}"
        // child segment should have a common prefix with segment[index]
        int idx = LongestCommonPrefix(child->segment_, segments[index]);
        if (idx != -1) {
          // matched
          std::string param_value = segments[index].substr(idx + 1);
          if (!param_value.empty()) {
            params.Set(child->wildcard_name_, param_value);
            found = child.get();
            break;
          }
        }
      }
    }
  }
  if (found) {
    return found->Search(url, segments, index + 1, params);
  }
  return nullptr;
}

}  // namespace detail

bool HTTPRouter::RegisterGet(HTTPCallback callback) {
  return Register(HTTPMethod::Get, std::move(callback));
}

bool HTTPRouter::RegisterPost(HTTPCallback callback) {
  return Register(HTTPMethod::Post, std::move(callback));
}

bool HTTPRouter::Register(HTTPMethod method, HTTPCallback&& callback) {
  return false;
}

}  // namespace http
}  // namespace ahrimq