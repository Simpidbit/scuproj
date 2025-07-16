#include <thread>
#include <vector>
#include <iostream>

void print(int n)
{
  for (int i = 0; i < 3; i++) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Hello " << n++ << std::endl;
  }
}

int main()
{
  int n = 10;
  std::vector<std::thread *> vec;

  // 5个线程
  std::thread *thptr;
  for (int i = 0; i < 5; i++) {
    thptr = new std::thread(&::print, n);
    vec.push_back(thptr);
  }

  for (auto each : vec) {
    each->join();
    delete each;
  }

  return 0;
}
