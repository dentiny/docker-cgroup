// This script keeps allocation memory via `mmap` to check whether memory constraint applies for mmap.

#include <cassert>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <cstdlib>
#include <thread>

#include <unistd.h>
#include <sys/mman.h>

int main() {
  const size_t page_size = sysconf(_SC_PAGESIZE);

  while (true) {
    void* addr = mmap(nullptr, page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(addr != MAP_FAILED);      
    memset(addr, 0, page_size);
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  return EXIT_SUCCESS;
}
