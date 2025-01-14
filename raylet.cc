// Mimic raylet, which runs forever.

#include <chrono>
#include <thread>

int main() {
  for (;;) {
    std::this_thread::sleep_for(std::chrono::seconds(3600));
  }
  return 0;
}
