#include "ahrimq/net/http/cookie.h"

#include <gtest/gtest.h>

TEST(CookieTest, ParseCookieStringTest) {
  std::string cks =
      "LIVE_BUVID=AUTO; buvid3=9EC714B9-6A15-4CD7-9EC0-BFCCA0C33E5B143074infoc; "
      "i-wanna-go-back=-1; buvid_fp_plain=undefined; CURRENT_BLACKGAP=0; "
      "nostalgia_conf=-1; hit-dyn-v2=1; blackside_state=0; DedeUserID=386873916; "
      "DedeUserID__ckMd5=b5785b613e045905; b_ut=5; "
      "buvid4=3835ACFC-794A-3D77-2306-07F5CC2696D548756-022082522-"
      "DGcwH0AsnIEluxrGzeGb7w==; b_nut=1661436048; "
      "rpdid=0zbfvUsqA6|1hskS6fOG|17|3w1P2MdA; hit-new-style-dyn=0; "
      "i-wanna-go-feeds=2; _uuid=A11DBC107-10D67-EBB1-B836-6AFD9F21CAEA60982infoc; "
      "CURRENT_QUALITY=80; fingerprint=1e64db3e69f87806b07938039815ceec; "
      "buvid_fp=1e64db3e69f87806b07938039815ceec; CURRENT_FNVAL=4048; "
      "SESSDATA=aa0f064a,1691379360,7d8eb*21; "
      "bili_jct=8ee196c823bf6184b9c06638e80ffd0b; sid=5lrho629; theme_style=light; "
      "b_lsid=B2A1D31A_1862F738422; innersign=0; "
      "bp_video_offset_386873916=760205663980748800";

  std::vector<ahrimq::http::Cookie> cookies;
  cookies.reserve(16);
  size_t n = ahrimq::http::ParseCookieString(cks, cookies);
  for (auto& cookie : cookies) {
    std::cout << cookie << '\n';
  }
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}