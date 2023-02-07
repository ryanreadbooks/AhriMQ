#include "circular_queue.hpp"

#include <gtest/gtest.h>

using namespace ahrimq;

TEST(CircularQueueTest, CapcityTest) {
  for (size_t i = 1; i <= 1024; i++) {
    detail::CircularQueue<int> que(i);
    EXPECT_EQ(que.Capacity(), i);
  }
}

struct Foo {
  Foo() = default;
  Foo(int d) : data(d) {}
  ~Foo() {}
  int data;
};

TEST(CircularQueueTest, PushTest) {
  detail::CircularQueue<Foo> que(5);
  for (int i = 0; i < 5; i++) {
    Foo f(i);
    que.Push(f);
    EXPECT_EQ(que.Size(), i + 1);
  }
  detail::CircularQueue<Foo> que2(100);
  for (int i = 0; i < 100; i++) {
    que2.Push(Foo(i + 1));
    EXPECT_EQ(que2.Size(), i + 1);
  }
}

TEST(CircularQueueTest, PopTest) {
  detail::CircularQueue<Foo> q(10);
  for (int i = 0; i < 10; i++) {
    Foo f(i);
    q.Push(f);
    EXPECT_EQ(q.Front().data, f.data);
    q.Pop();
    EXPECT_EQ(q.Size(), 0);
  }
  detail::CircularQueue<Foo> q2(100);
  for (int i = 0; i < 100; i++) {
    q.Push(Foo(i));
    EXPECT_EQ(q.Front().data, i);
    q.Pop();
    EXPECT_TRUE(q.Empty());
  }
}

TEST(CircularQueueTest, PushPopTest) {
  detail::CircularQueue<Foo> q(5);
  q.Push(Foo(1));
  q.Push(Foo(2));
  q.Push(Foo(3));
  q.Push(Foo(4));
  q.Push(Foo(5));
  EXPECT_THROW(q.Push(6), std::out_of_range);
  EXPECT_EQ(q.Size(), 5);
  q.Pop();
  q.Pop();
  q.Pop();
  EXPECT_EQ(q.Size(), 2);
  q.Pop();
  q.Pop();
  EXPECT_EQ(q.Size(), 0);
  EXPECT_THROW(q.Pop(), std::out_of_range);
  q.Push(100);
  q.Pop();
  EXPECT_THROW(q.Front(), std::out_of_range);
  q.Push(200);
  EXPECT_EQ(q.Front().data, 200);
  q.Push(300);
  EXPECT_EQ(q.Front().data, 200);
  q.Push(400);
  q.Push(500);
  EXPECT_EQ(q.Front().data, 200);
  q.Pop();
  q.Pop();
  EXPECT_EQ(q.Front().data, 400);
  EXPECT_EQ(q.Size(), 2);
}

struct Bar {
  ~Bar() {
    times++;
  }
  static size_t times;
};
size_t Bar::times = 0;

TEST(CircularQueueTest, DestructorTest) {
  struct testStruct {
    int cap;
    int num;
    int dest;
  };

  std::vector<testStruct> cases{
      {60, 50, 23}, {100, 100, 34},  {34, 7, 1},   {2345, 1265, 908}, {14, 2, 2},
      {95, 50, 49}, {600, 503, 231}, {70, 69, 69}, {80, 79, 78},
  };

  Bar b;
  for (const auto& c : cases) {
    {
      {
        detail::CircularQueue<Bar> q(c.cap);
        for (int i = 0; i < c.num; i++) {
          q.Push(b);
        }
        for (int i = 0; i < c.dest; i++) {
          q.Pop();
        }
        EXPECT_EQ(q.Size(), c.num - c.dest);
      }
      EXPECT_EQ(Bar::times, c.num);  // record Bar::~Bar invokation count
    }
    Bar::times = 0;
  }
}

struct StructWithMemberInHeap {
  explicit StructWithMemberInHeap(int i) : data(new int(i)) {}

  StructWithMemberInHeap(StructWithMemberInHeap&& o) {
    data = o.data;
    o.data = nullptr;
  }

  StructWithMemberInHeap(const StructWithMemberInHeap& o) {
    data = new int(*o.data);
  }

  StructWithMemberInHeap& operator=(const StructWithMemberInHeap& o) {
    if (this == &o) {
      return *this;
    }
    if (data != nullptr) {
      // free existing allocated space
      delete data;
    }
    data = new int(*o.data);
    return *this;
  }

  ~StructWithMemberInHeap() {
    delete data;
    data = nullptr;
  }

  bool operator==(const StructWithMemberInHeap& b) {
    if (data == nullptr || b.data == nullptr) {
      return false;
    }
    return *data == *b.data;
  }

  int* data = nullptr;
};
using test_t = StructWithMemberInHeap;

TEST(CircularQueueTest, TestWithStructInHeap) {
  // should not have memory leak
  detail::CircularQueue<test_t> q(105);
  for (int i = 0; i < 100; i++) {
    q.Push(test_t(i));
  }
  test_t t(100);
  for (int i = 0; i < 5; i++) {
    q.Push(t);
  }
  EXPECT_EQ(q.Size(), 105);

  // pop
  {
    detail::CircularQueue<test_t> q2(5);
    q2.Push(test_t(1));
    q2.Push(test_t(1));
    q2.Push(test_t(1));
    q2.Push(test_t(1));
    q2.Pop();
    q2.Pop();
    q2.Push(test_t(2));
    q2.Push(test_t(2));
    q2.Push(test_t(2));
    EXPECT_THROW(q2.Push(test_t(2)), std::out_of_range);
    EXPECT_THROW(q2.Push(test_t(2)), std::out_of_range);
    EXPECT_THROW(q2.Push(test_t(2)), std::out_of_range);
    EXPECT_THROW(q2.Push(test_t(2)), std::out_of_range);
    EXPECT_THROW(q2.Push(test_t(2)), std::out_of_range);
    EXPECT_THROW(q2.Push(test_t(2)), std::out_of_range);
    EXPECT_THROW(q2.Push(test_t(2)), std::out_of_range);
    q2.Pop();
    q2.Pop();
    q2.Pop();
    q2.Pop();
    q2.Pop();
    EXPECT_THROW(q2.Pop(), std::out_of_range);
    EXPECT_THROW(q2.Pop(), std::out_of_range);
    EXPECT_THROW(q2.Pop(), std::out_of_range);
    EXPECT_THROW(q2.Pop(), std::out_of_range);
    EXPECT_THROW(q2.Pop(), std::out_of_range);
    q2.Push(t);
    EXPECT_EQ(q2.Size(), 1);
  }
}

TEST(CircularQueueTest, EmplaceTest) {
  detail::CircularQueue<test_t> q(10);
  for (int i = 0; i < 10; i++) {
    q.Emplace(i);
  }
  int i = 0;
  while (!q.Empty()) {
    EXPECT_EQ(*q.Front().data, i++);
    q.Pop();
  }

  struct testC {
    testC(int i, double d, const std::string& s) : i(i), d(d), str(s) {}

    int i;
    double d;
    std::string str;
  };

  struct testCase {
    int cap;
    int num;
    int ci;
    double cd;
    std::string cstr;
  };

  std::vector<testCase> cases{
      {100, 23, 18, 0.23, "HELLOWORLD"},
      {9, 6, 1, 10.9034, "are you aweradfoihzkjv s we4r5a1sd3f1ds"},
      {15, 10, 9, 96.452, "123456790zhdksdweroieur"},
      {39, 39, 15, 10.9034, "1230+--++!~!!##@%$^&%^%&%^&5"},
      {56, 50, 45678219, 300239.3693695, "werwsfqeokmnjtqwdvcvwqeq"},
      {71, 63, 263232323, 60.369, " 4r5a1sdyou awareeradfoihzkjv s we3f1ds"},
  };

  for (auto&& c : cases) {
    detail::CircularQueue<testC> q(c.cap);
    for (int i = 0; i < c.num; i++) {
      q.Emplace(c.ci, c.cd, c.cstr);
      EXPECT_DOUBLE_EQ(q.Front().d, c.cd);
      EXPECT_EQ(q.Front().i, c.ci);
      EXPECT_STREQ(q.Front().str.c_str(), c.cstr.c_str());
      q.Pop();
    }
  }
}

TEST(CircularQueueTest, TestCopyConstructor) {
  detail::CircularQueue<test_t> q(10);
  for (size_t i = 0; i < 8; i++) {
    q.Push(test_t(i));
  }
  for (size_t i = 0; i < 3; i++) {
    q.Pop();
  }
  for (size_t i = 10; i < 13; i++) {
    q.Push(test_t(i));
  }
  detail::CircularQueue<test_t> q2 = q;
  detail::CircularQueue<test_t> q3(q2);
  // check q q2 q3 the same
  EXPECT_EQ(q.Size(), q2.Size());
  EXPECT_EQ(q2.Size(), q3.Size());
  EXPECT_EQ(q.Capacity(), q2.Capacity());
  EXPECT_EQ(q2.Capacity(), q3.Capacity());
  std::vector<test_t> vq, vq2, vq3;
  vq.reserve(q.Size());
  vq2.reserve(q.Size());
  vq3.reserve(q.Size());

  while (q.Empty()) {
    vq.push_back(q.Front());
    vq2.push_back(q2.Front());
    vq3.push_back(q3.Front());
    q.Pop();
    q2.Pop();
    q3.Pop();
  }
  EXPECT_EQ(vq.size(), vq2.size());
  EXPECT_EQ(vq3.size(), vq3.size());

  for (size_t i = 0; i < vq.size(); i++) {
    EXPECT_TRUE(vq[i] == vq2[i]);
    EXPECT_TRUE(vq3[i] == vq2[i]);
  }
}
TEST(CircularQueueTest, TestCopyConstructorWithString) {
  detail::CircularQueue<std::string> q(10);
  for (size_t i = 0; i < 8; i++) {
    q.Push(std::to_string(i) + "abcdefghijklomnokowepoaid");
  }
  for (size_t i = 0; i < 3; i++) {
    q.Pop();
  }
  for (size_t i = 10; i < 13; i++) {
    q.Push(std::to_string(i) + "ADSDWESDWEeger+961t23");
  }

  detail::CircularQueue<std::string> q2 = q;
  detail::CircularQueue<std::string> q3(q2);
  // check q q2 q3 the same
  EXPECT_EQ(q.Size(), q2.Size());
  EXPECT_EQ(q2.Size(), q3.Size());
  EXPECT_EQ(q.Capacity(), q2.Capacity());
  EXPECT_EQ(q2.Capacity(), q3.Capacity());

  std::vector<std::string> vq, vq2, vq3;
  vq.reserve(q.Size());
  vq2.reserve(q.Size());
  vq3.reserve(q.Size());

  while (q.Empty()) {
    vq.push_back(q.Front());
    vq2.push_back(q2.Front());
    vq3.push_back(q3.Front());
    q.Pop();
    q2.Pop();
    q3.Pop();
  }
  EXPECT_EQ(vq.size(), vq2.size());
  EXPECT_EQ(vq3.size(), vq3.size());

  for (size_t i = 0; i < vq.size(); i++) {
    EXPECT_TRUE(vq[i] == vq2[i]);
    EXPECT_TRUE(vq3[i] == vq2[i]);
  }
}

TEST(CircularQueueTest, TestCopyAssignment) {
  detail::CircularQueue<test_t> q(10);
  for (size_t i = 0; i < 10; i++) {
    q.Push(test_t(i));
  }
  for (size_t i = 0; i < 4; i++) {
    q.Pop();
  }
  for (size_t i = 10; i < 13; i++) {
    q.Push(test_t(i));
  }

  detail::CircularQueue<test_t> q2(16);
  detail::CircularQueue<test_t> q3(8);

  // copy assignment
  q2 = q;
  q3 = q2;

  // check q q2 q3 the same
  EXPECT_EQ(q.Size(), q2.Size());
  EXPECT_EQ(q2.Size(), q3.Size());
  std::vector<test_t> vq, vq2, vq3;
  vq.reserve(q.Size());
  vq2.reserve(q.Size());
  vq3.reserve(q.Size());

  while (q.Empty()) {
    vq.push_back(q.Front());
    vq2.push_back(q2.Front());
    vq3.push_back(q3.Front());
    q.Pop();
    q2.Pop();
    q3.Pop();
  }
  EXPECT_EQ(vq.size(), vq2.size());
  EXPECT_EQ(vq3.size(), vq3.size());

  for (size_t i = 0; i < vq.size(); i++) {
    EXPECT_TRUE(vq[i] == vq2[i]);
    EXPECT_TRUE(vq3[i] == vq2[i]);
  }
}

TEST(CircularQueueTest, TestCopyAssignmentWitString) {
  detail::CircularQueue<std::string> q(1024);
  std::string temp = "0123456789ABCDEF";
  for (size_t i = 0; i < 1000; i++) {
    std::string rnd;
    for (size_t j = 0; j < i; j++) {
      rnd += temp[j % 15];
    }
    std::string value = rnd + std::to_string(i);
    q.Push(value);
  }
  for (size_t i = 0; i < 500; i++) {
    q.Pop();
  }
  for (size_t i = 10; i < 24; i++) {
    q.Push(std::string("aaaaaaaaaaaaaaaeeeeeWEeeeeeeeeeeeeeee") + std::to_string(i));
  }

  detail::CircularQueue<std::string> q2(500);
  detail::CircularQueue<std::string> q3(5);

  // copy assignment
  q2 = q;
  q3 = q2;

  // check q q2 q3 the same
  EXPECT_EQ(q.Size(), q2.Size());
  EXPECT_EQ(q2.Size(), q3.Size());
  std::vector<std::string> vq, vq2, vq3;
  vq.reserve(q.Size());
  vq2.reserve(q.Size());
  vq3.reserve(q.Size());

  while (q.Empty()) {
    vq.push_back(q.Front());
    vq2.push_back(q2.Front());
    vq3.push_back(q3.Front());
    q.Pop();
    q2.Pop();
    q3.Pop();
  }
  EXPECT_EQ(vq.size(), vq2.size());
  EXPECT_EQ(vq3.size(), vq3.size());

  for (size_t i = 0; i < vq.size(); i++) {
    EXPECT_TRUE(vq[i] == vq2[i]);
    EXPECT_TRUE(vq3[i] == vq2[i]);
  }
}

TEST(CircularQueueTest, TestMoveConstructor) {
  detail::CircularQueue<test_t> q(10);
  for (size_t i = 0; i < 10; i++) {
    q.Push(test_t(i));
  }
  for (size_t i = 0; i < 4; i++) {
    q.Pop();
  }
  for (size_t i = 10; i < 13; i++) {
    q.Push(test_t(i));
  }

  detail::CircularQueue<test_t> q2(std::move(q));
  EXPECT_EQ(q.Size(), 0);
  EXPECT_EQ(q2.Size(), 9);
  EXPECT_EQ(q.Capacity(), 0);
  EXPECT_EQ(q2.Capacity(), 10);
  EXPECT_THROW(q.Pop(), std::out_of_range);
  EXPECT_THROW(q.Push(test_t(1)), std::out_of_range);
  EXPECT_TRUE(q.Empty());
  q2.Push(test_t(2));
  EXPECT_TRUE(q2.Full());

  detail::CircularQueue<test_t> q3(std::move(q2));
  EXPECT_EQ(q2.Size(), 0);
  EXPECT_EQ(q3.Size(), 10);
  EXPECT_EQ(q2.Capacity(), 0);
  EXPECT_EQ(q3.Capacity(), 10);
  EXPECT_THROW(q2.Pop(), std::out_of_range);
  EXPECT_THROW(q2.Push(test_t(1)), std::out_of_range);
  EXPECT_TRUE(q2.Empty());
  EXPECT_TRUE(q3.Full());
  
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}