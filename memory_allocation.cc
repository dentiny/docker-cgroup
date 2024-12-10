// This process should executed as background process.
//
// The script allocates 1MiB every second, 1GiB takes 1024 sec.

#include <chrono>
#include <thread>
#include <vector>

int main() {
  std::vector<std::vector<char>> blocks;
  for (;;) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::vector<char> new_block(1024 * 1024, '0');
    blocks.emplace_back(std::move(new_block));
  }
  return 0;
}
