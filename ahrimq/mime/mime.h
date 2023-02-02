#ifndef _MIME_H_
#define _MIME_H_

#include <string>

namespace ahrimq {

namespace mime {

std::string DecideMimeTypeFromExtension(const std::string& path);

}  // namespace mime

}  // namespace ahrimq

#endif  // _MIME_H_