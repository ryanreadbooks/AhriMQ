#include "buffer.h"

#include <gtest/gtest.h>

#include <iostream>

auto show_buffer = [](ahrimq::Buffer &s) {
  for (auto &c : s.ReadableAsString()) {
    if (c == '\r') {
      std::cout << "\\r";
    } else if (c == '\n') {
      std::cout << "\\n";
    } else {
      std::cout << c;
    }
  }
  std::cout << std::endl;
};

TEST(BufferTest, BasicTest) {
  ahrimq::Buffer buf(8);

  std::string s5 = "hell0";
  buf.Append(s5);
  std::string s6 = "wonder";
  buf.Append(s6);

  char ch7[7] = {'1', '2', '3', '4', '5', '6', '7'};

  EXPECT_EQ(buf.ReadableAsString(), "hell0wonder");
  buf.Append(ch7, 7);
  EXPECT_EQ(buf.ReadableAsString(), "hell0wonder1234567");

  buf.ReaderIdxForward(14);
  EXPECT_EQ(buf.ReadableAsString(), "4567");

  std::string s7 = "qwertyu";

  buf.Append(s7);
  EXPECT_EQ(buf.ReadableAsString(), "4567qwertyu");

  ahrimq::Buffer buf2(10);
  std::string templatestr = "123456";
  for (int i = 0; i < 6; i++) {
    char buf[6] = {'1', '2', '3', '4', '5', '6'};
    buf2.Append(buf, 6);
    std::string ss = templatestr;
    for (int j = 0; j < i; j++) {
      ss += templatestr;
    }
    EXPECT_EQ(buf2.ReadableAsString(), ss);
  }
};

TEST(BufferTest, BasicTest2) {
  ahrimq::Buffer buf3(4);
  for (int i = 0; i < 5; i++) {
    char buf[5] = {'1', '2', '3', '4', '5'};
    buf3.Append(buf, 5);
    show_buffer(buf3);
    if (i == 0) {
      EXPECT_EQ(buf3.ReadableAsString(), "12345");
    } else if (i == 1) {
      EXPECT_EQ(buf3.ReadableAsString(), "4512345");
    } else if (i == 2) {
      EXPECT_EQ(buf3.ReadableAsString(), "234512345");
    } else if (i == 3) {
      EXPECT_EQ(buf3.ReadableAsString(), "51234512345");
    } else if (i == 4) {
      EXPECT_EQ(buf3.ReadableAsString(), "3451234512345");
    }
    buf3.ReaderIdxForward(3);
  }
}

TEST(BufferTest, CopyTest) {
  ahrimq::Buffer buf;
  buf.Append("helloworld!!");
  std::vector<char> copy;
  int n = buf.Size();
  copy.reserve(n);
  copy.insert(copy.begin(), buf.BeginReadPointer(), buf.BeginReadPointer() + n);
  std::string s(copy.begin(), copy.end());
  ASSERT_STREQ(s.c_str(), "helloworld!!");
}

TEST(BufferTest, TrimLeftTest) {
  ahrimq::Buffer buf;
  buf.Append("\thelloworld");
  ASSERT_EQ(buf.Size(), 11);
  buf.TrimLeft();
  ASSERT_EQ(buf.BeginRead(), 1);
  ASSERT_STREQ(buf.ReadableAsString().c_str(), "helloworld");
  ASSERT_EQ(buf.Size(), 10);
  buf.ReadAll();
  buf.Append("   are you ok??");
  buf.TrimLeft();
  ASSERT_STREQ(buf.ReadableAsString().c_str(), "are you ok??");
  ASSERT_EQ(buf.Size(), 12);
  buf.ReadAll();
  buf.Append("  \t\r\n \r\r\n\t");
  buf.TrimLeft();
  ASSERT_EQ(0, buf.Size());
  buf.Append("this is it!!!");
  ASSERT_EQ(13, buf.Size());
  buf.TrimLeft();
  ASSERT_EQ(13, buf.Size());
  ahrimq::Buffer buf2;
  buf2.Append(
      "                  \t\t\t\t\t\n\n\n\n\n\r\r\r\r\t\t\t\n                    "
      "wefwefsafawerw+9a46aaeraadfewrpafbvzzljfwaffsd1f3sf0sasdf3adf16we4aw6++++++++"
      "+++++++++++++++--------------f65adf");
  buf2.TrimLeft();
  std::string tmp =
      "wefwefsafawerw+9a46aaeraadfewrpafbvzzljfwaffsd1f3sf0sasdf3adf16we4aw6++++++++"
      "+++++++++++++++--------------f65adf";
  ASSERT_STREQ(tmp.c_str(), buf2.ReadableAsString().c_str());
}

TEST(BufferTest, TrimRightTest) {
  ahrimq::Buffer buf(64);
  buf.Append("helloworld\n\n ");
  ASSERT_EQ(buf.Size(), 13);
  buf.TrimRight();
  ASSERT_EQ(buf.Size(), 10);
  ASSERT_STREQ(buf.ReadableAsString().c_str(), "helloworld");
  buf.Append("12345678910");
  ASSERT_STREQ(buf.ReadableAsString().c_str(), "helloworld12345678910");
  ASSERT_EQ(buf.Size(), 21);
  ASSERT_EQ(buf.Capacity(), 64);

  ahrimq::Buffer buf2;
  buf2.Append(
      "sdfowpehspdfidspafsipad apf "
      "rp238qr7yaofh92qfdafasdfsd+4f+asd5fas+d5f1a+sdf4d+avad1f+asdf5+ads5f         "
      "                  \t\t\t\t\t\t\t\n\n\n  \n\n\r\r\r   \r\r\t\n\r     \n");
  std::string tmp =
      "sdfowpehspdfidspafsipad apf "
      "rp238qr7yaofh92qfdafasdfsd+4f+asd5fas+d5f1a+sdf4d+avad1f+asdf5+ads5f";
  buf2.TrimRight();
  buf2.TrimLeft();
  ASSERT_STREQ(buf2.ReadableAsString().c_str(), tmp.c_str());
  ASSERT_EQ(buf2.Size(), tmp.size());
}

TEST(BufferTest, TrimLeftRightTest) {
  ahrimq::Buffer buf;
  buf.Append("            \r\nhelloworld\r\n            \r\r\r\r\n");
  buf.TrimLeftRight();
  ASSERT_EQ(buf.Size(), 10);
  buf.Append("\r\n                  ");
  buf.TrimLeftRight();
  ASSERT_EQ(buf.Size(), 10);
  buf.Append(
      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n\n\n    ");
  buf.TrimLeftRight();
  ASSERT_EQ(buf.Size(), 10 + 60);
  ASSERT_STREQ(
      "helloworldaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
      buf.ReadableAsString().c_str());
}

TEST(BufferTest, CRLFTest) {
  ahrimq::Buffer buf;
  buf.Append("\r\n");
  std::string s = buf.ReadStringAndForwardTill("\r\n");
  EXPECT_TRUE(s.empty());
}

TEST(BufferTest, AppendBufferTest) {
  ahrimq::Buffer buf;
  buf.Append("helloworldxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
  ahrimq::Buffer buf2;
  buf2.Append(", are you ok?kkkkkkkkkkkkkkkkkkkkkkkkkkkk");
  buf.Append(buf2);
  EXPECT_EQ(buf.ReadableAsString(),
            "helloworldxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx, are you "
            "ok?kkkkkkkkkkkkkkkkkkkkkkkkkkkk");
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}