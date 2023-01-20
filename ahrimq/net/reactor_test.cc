#include "reactor.h"

#include <iostream>

int main() {
  ahrimq::Reactor reactor("127.0.0.1", 9527, 4);
  reactor.React();
  reactor.Wait();

  return 0;
}