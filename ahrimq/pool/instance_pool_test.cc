#include "instance_pool.hpp"

#include <gtest/gtest.h>

using namespace ahrimq;

struct Obj {
  Obj(int i, const std::string& str) : data(i), s(str){};

  ~Obj() {
    std::cout << "Obj::~Obj\n";
  };

 private:
  int data;
  std::string s;
};

TEST(InstancePoolTest, InstancePoolImplTest) {
  InstancePool<Obj> pool1(10);
  std::vector<InstancePool<Obj>::PoolPtr> ps;
  for (size_t i = 0; i < 15; i++) {
    ps.emplace_back(pool1.Construct(i, "helloworldHELLOWORLD?!+-*/"));
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}