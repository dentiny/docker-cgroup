// This script setup prerequisites for cgroup memory and CPU subtree control.

#include <fcntl.h>
#include <sys/stat.h>

#include <cassert>
#include <fstream>
#include <string>

namespace {

constexpr mode_t kReadWritePerm = S_IRUSR | S_IWUSR;

void MakeDirectory(const std::string &directory) {
  int ret_code = mkdir(directory.data(), kReadWritePerm);
  assert(ret_code == 0 || errno == EEXIST);
}

void MoveProcsBetweenCgroups(const std::string &from, const std::string &to) {
  std::ifstream in_file(from.data());
  assert(in_file.good());
  std::ofstream out_file(to.data(), std::ios::app | std::ios::out);
  assert(out_file.good());

  pid_t pid = 0;
  while (in_file >> pid) {
    out_file << pid << std::endl;
  }
  out_file.flush();
  assert(out_file.good());
}

void EnableCgroupSubtreeControl(const std::string &subtree_control_path) {
  std::ofstream out_file(subtree_control_path, std::ios::app | std::ios::out);
  assert(out_file.good());

  out_file << "+memory";
  out_file.flush();
  assert(out_file.good());

  out_file << "+cpu";
  out_file.flush();
  assert(out_file.good());
}

}  // namespace

int main(int argc, char** argv) {
  const std::string node_cgroup = "/sys/fs/cgroup/ray_node";
  const std::string node_system_cgroup = "/sys/fs/cgroup/ray_node/system";

  const std::string root_cgroup_proc_file = "/sys/fs/cgroup/cgroup.procs";
  const std::string node_system_cgroup_proc_file = "/sys/fs/cgroup/ray_node/system/cgroup.procs";

  const std::string node_cgroup_subtree_control = "/sys/fs/cgroup/ray_node/cgroup.subtree_control";

  MakeDirectory(node_cgroup);
  MakeDirectory(node_system_cgroup);
  MoveProcsBetweenCgroups(root_cgroup_proc_file, node_system_cgroup_proc_file);
  EnableCgroupSubtreeControl(node_cgroup_subtree_control);

  return 0;
}
