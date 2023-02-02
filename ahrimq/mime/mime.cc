#include "mime/mime.h"

#include "base/str_utils.h"

namespace ahrimq {

namespace mime {

#define SCEQ1(src, ret)         \
  if (StrCaseEqual(ext, src)) { \
    return ret;                 \
  }

#define SCEQ2(src1, src2, ret)                              \
  if (StrCaseEqual(ext, src1) || StrCaseEqual(ext, src2)) { \
    return ret;                                             \
  }

std::string DecideMimeTypeFromExtension(const std::string& path) {
  std::string ext = FileExtension(path);
  // text
  SCEQ2(".html", ".htm", "text/html")
  SCEQ2(".js", ".mjs", "text/javascript")
  SCEQ1(".css", "text/css")
  SCEQ1(".csv", "text/csv")
  SCEQ1(".txt", "text/plain")

  // image
  SCEQ1(".png", "image/png")
  SCEQ1(".webp", "image/webp")
  SCEQ1(".gif", "image/gif")
  SCEQ1(".ico", "image/vnd.microsoft.icon")
  SCEQ1(".svg", "image/svg+xml")
  SCEQ2(".bmp", ".dib", "image/bmp")
  SCEQ2(".jpg", ".jpeg", "image/jpeg")
  SCEQ2(".tif", ".tiff", "image/tiff")

  // application
  SCEQ1(".bin", "application/octet-stream")
  SCEQ1(".bz", "application/x-bzip")
  SCEQ1(".bz2", "application/x-bzip2")
  SCEQ1(".json", "application/json")
  SCEQ1(".pdf", "application/pdf")
  SCEQ1(".rar", "application/x-rar-compressed")
  SCEQ1(".sh", "application/x-sh")
  SCEQ1(".tar", "application/x-tar")
  SCEQ1(".zip", "application/zip")
  SCEQ1(".7z", "application/x-7z-compressed")
  SCEQ1(".xml", "application/xml")
}

}  // namespace mime

}  // namespace ahrimq