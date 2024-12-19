# docker-cgroup
This repository is used to check cgroup operation within a container.

### Steps to reproduce docker experiment
1. Make sure host machine already mounts cgroupv2 with rw permission
```shell
sudo vim /etc/default/grub
set GRUB_CMDLINE_LINUX="systemd.unified_cgroup_hierarchy=1"
sudo update-grub
sudo reboot
```
2. Create a docker with ubuntu image, start and enter into the container
```shell
docker pull ubuntu:20.04
docker run --security-opt seccomp=unconfined --privileged -it ubuntu:20.04
```
3. Create application related cgroups
```shell
# Create our own system cgroup
mkdir -p /sys/fs/cgroup/ray_system_uuid
# Move all root cgroup process into the system cgroup
echo 1 > /sys/fs/cgroup/ray_system_uuid/cgroup.procs
# Specify memory and CPU enforcement for the current cgroup, which allows self-customized cgroup
echo "+memory +pids" >> /sys/fs/cgroup/cgroup.subtree_control
# Set memory consumption limitation for the system cgroup
echo 100G > /sys/fs/cgroup/ray_system_uuid/memory.max

# Create our own application cgroup
mkdir -p /sys/fs/cgroup/ray_application_cgroup
# Specify memory min and max for the cgroup
echo 80M > /sys/fs/cgroup/ray_application_cgroup/memory.max
# Allow resource enforcement on leaf cgroup
echo "+memory +pids" >> /sys/fs/cgroup/ray_application_cgroup/cgroup.subtree_control

# Create leaf cgroup for each application groups
mkdir -p /sys/fs/cgroup/ray_application_cgroup/uuid
# Create `procs` file, which indicates the process under the current cgroup
echo pid >> /sys/fs/cgroup/ray_application_cgroup/uuid/cgroup.procs
# Specify memory min and max for the cgroup
echo 100M > /sys/fs/cgroup/ray_application_cgroup/uuid/memory.max
```
4. Appendix
```shell
# Check cgroupv2 mount
mount | grep cgroup
# Install useful tools
apt update -y && apt install -y htop g++ vim
# Compile test program
g++ -std=c++17 memory_allocation.cc -o memory_allocation
# Execute test program in background
./memory_allocation &
# Check current cgroup memory consumption
cat /sys/fs/cgroup/ray_application_cgroup/memory.current
# Check current process memory consumption
root@3ee2f6db14d7:/# cat /proc/943/status | grep -iE 'vmrss|vmsize'
# Check CRI
k get nodes -o wide
```
5. Observation / takeaway
- For `memory_allocation` program, if we update `memory.max` to be less than current consumption, memory will be released and executes normally
- Before adding memory limit to leaf cgroup leaf, should apply suitable permission at parent node's `cgroup.subtree_control`

### Steps to reproduce kubernetes experiment
1. Create local kubernetes cluster
```shell
kind create cluster --image=kindest/node:v1.26.0
```
2. Build image and load to kubernetes cluster
```shell
docker build -t dentiny:stall_image .
# `kind` is the cluster name, could be checked by `kind get clusters`
kind load docker-image dentiny:stall_image --name kind
# Deploy k8s deployment
k apply -f deployment.yaml
```
