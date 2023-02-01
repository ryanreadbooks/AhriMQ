#include "buffer.h"

#include <iostream>

#include <gtest/gtest.h>

TEST(BufferTest, CopyTest) {
  ahrimq::Buffer buf;
  buf.Append("helloworld!!");
  std::vector<char> copy;
  int n = buf.Size();
  copy.reserve(n);
  copy.insert(copy.begin(), buf.BeginReadIndex(), buf.BeginReadIndex() + n);
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
  ahrimq::Buffer buf;
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
  buf.Append("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n\n\n    ");
  buf.TrimLeftRight();
  ASSERT_EQ(buf.Size(), 10 + 60);
  ASSERT_STREQ("helloworldaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", buf.ReadableAsString().c_str());
}

TEST(BufferTest, CRLFTest) {
  ahrimq::Buffer buf;
  buf.Append("\r\n");
  std::string s = buf.ReadStringAndForwardTill("\r\n");
  if (s.empty()) {
    std::cout << "s is empty\n";
  }
  std::cout << buf.ReadableAsString() << std::endl;
  std::cout << "-------\n";
}

TEST(BufferTest, AppendBufferTest) {
  ahrimq::Buffer buf;
  buf.Append("helloworld");
  ahrimq::Buffer buf2;
  buf2.Append(", are you ok?");
  buf.Append(buf2);
  std::cout<< buf.ReadableAsString() << std::endl;
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}