// This program stalls forever, used to check kubernetes related configs.

#include <chrono>
#include <thread>

int main() {
  for (;;) {
    std::this_thread::sleep_for(std::chrono::seconds(18000));
  }
  return 0;
}
