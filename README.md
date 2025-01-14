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
# (to verify cgroup operation at non-docker env) At the very beginning create a few processes.
./raylet &
# Check the process is added into default operating system's cgroup.
cat /sys/fs/cgroup/cgroup.procs

# Create operating system's cgroup.
mkdir -p /sys/fs/cgroup/operating_system
# Move our existing processes to the default cgroup.
# Notice since init process is placed under `operating_system` cgroup, new processes spawned will by default placed under certain cgroup.
for pid in $(cat /sys/fs/cgroup/cgroup.procs); do
    echo $pid >> /sys/fs/cgroup/operating_system/cgroup.procs
done
# Specify memory and CPU enforcement for the current cgroup, which allows self-customized cgroup.
# Subcgroups are allowed to set memory related parameters as well.
echo "+memory" >> /sys/fs/cgroup/cgroup.subtree_control

# Create our own ray cgroup
mkdir -p /sys/fs/cgroup/ray_cluster
# Set memory consumption limitation for the system cgroup
echo 100G > /sys/fs/cgroup/ray_cluster/memory.max
# Also allow ray system and application cgroup to configure their own memory parameters
echo "+memory" >> /sys/fs/cgroup/ray_cluster/cgroup.subtree_control 

# Create our own ray system cgroup
mkdir -p /sys/fs/cgroup/ray_cluster/internal
# Place ray internal components into system cgroup
echo $raylet_pid >> /sys/fs/cgroup/ray_cluster/internal/cgroup.procs

# Create our own application cgroup
mkdir -p /sys/fs/cgroup/ray_cluster/application
# Specify memory min and max for the cgroup
echo 80M > /sys/fs/cgroup/ray_cluster/application/memory.max
# Allow resource enforcement on leaf cgroup
echo "+memory" >> /sys/fs/cgroup/ray_cluster/application/cgroup.subtree_control

# Create leaf cgroup for each application groups
mkdir -p /sys/fs/cgroup/ray_cluster/application/task_id
# Create `procs` file, which indicates the process under the current cgroup
echo pid >> /sys/fs/cgroup/ray_cluster/application/task_id/cgroup.procs
# Specify memory min and max for the cgroup
echo 100M > /sys/fs/cgroup/ray_cluster/application/task_id/memory.max

# Destruct the application specific cgroup.
#
# Create default application cgroup.
mkdir -p /sys/fs/cgroup/ray_cluster/application/default
# Move our own application to the default cgroup.
for pid in $(cat /sys/fs/cgroup/ray_cluster/application/uuid/cgroup.procs); do
    echo $pid >> /sys/fs/cgroup/ray_cluster/application/default/cgroup.procs
done
# Delete application cgroup folder.
# Notice we cannot `rm -rf` but have to `rmdir` an empty cgroup folder.
rmdir /sys/fs/cgroup/ray_cluster/application/task_id

# Place process back from default cgroup to specific cgroup.
#
# Create a specific cgroup.
mkdir /sys/fs/cgroup/ray_application_cgroup/task_id
# Put process into the cgroup.
echo pid >> /sys/fs/cgroup/ray_application_cgroup/uuid/cgroup.procs
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
# Cleanup unused docker image and others
docker system prune -a --volumes
```
5. Observation / takeaway
- For `memory_allocation` program, if we update `memory.max` to be less than current consumption, memory will be released and executes normally
- Before adding memory limit to leaf cgroup leaf, should apply suitable permission at parent node's `cgroup.subtree_control`
- Only allow to add PIDs into leaf cgroup node
- Cgroup can only be deleted when no pids managed
- Only allow to update child cgroup subcontrol parent cgroup doesn't contain any PIDs

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
