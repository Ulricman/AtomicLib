#include <functional>
#include <iostream>
#include <new>
#include <thread>

#include "atomiclib/ringbuffer.hpp"

int main() {
  lockfreelib::RingBuffer<std::function<void(void)>> buffer(100);

  int numTasks = 100, numThreads = 10;
  for (int i = 0; i < numTasks; ++i) {
    auto func = [i]() { std::cout << i << std::endl; };
    while (!buffer.push(func));
  }
  std::function<void()> func;
  for (int i = 0; i < numTasks; ++i) {
    while (!buffer.pop(func));
    func();
  }
}