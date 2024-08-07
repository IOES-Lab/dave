#!/bin/bash
# Script to install development tools and libraries for robotics and simulation
echo
echo -e "\033[94m============================================================\033[0m"
echo -e "\033[94m== One-liner Installation Script for ROS-Gazebo Framework ==\033[0m"
echo -e "\033[94m============================================================\033[0m"
echo -e "Requirements: Ubuntu 24.04 LTS Noble"
echo -e "\033[94m============================================================\033[0m"

echo
echo -e "\033[96m(1/4) -------------    Updating the System  ----------------\033[0m"
echo "Performing full system upgrade (this might take a while)..."
sudo sudo apt update && apt full-upgrade -y

echo
echo -e "\033[96m(2/4) ------------    Install Dependencies   ---------------\033[0m"
echo -e "\033[34mInstalling essential tools and libraries...\033[0m"
sudo apt install -y \
    build-essential \
    cmake \
    cppcheck \
    curl \
    git \
    gnupg \
    libeigen3-dev \
    libgles2-mesa-dev \
    lsb-release \
    pkg-config \
    protobuf-compiler \
    python3-dbg \
    python3-pip \
    python3-venv \
    qtbase5-dev \
    ruby \
    software-properties-common \
    sudo \
    cppzmq-dev \
    wget

echo
echo -e "\033[96m(3/4) ------------    Install Package Keys   ---------------\033[0m"
echo -e "\033[34mInstalling Signing Keys for ROS and Gazebo...\033[0m"
# Remove keyring if exists to avoid conflicts
sudo rm -f /usr/share/keyrings/ros2-latest-archive-keyring.gpg && \
    sudo rm -rf /etc/apt/sources.list.d/ros2-latest.list
# Get Keys
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg
sudo wget https://packages.osrfoundation.org/gazebo.gpg \
    -O /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg

ARCH=$(dpkg --print-architecture)
# shellcheck disable=SC1091
UBUNTU_CODENAME=$( . /etc/os-release && echo "$UBUNTU_CODENAME")
REPO="deb [arch=$ARCH signed-by=/usr/share/keyrings/ros-archive-keyring.gpg] \
http://packages.ros.org/ros2/ubuntu $UBUNTU_CODENAME main"
echo "$REPO" | sudo tee /etc/apt/sources.list.d/ros2.list >/dev/null

DISTRO=$(lsb_release -cs)
REPO="deb [arch=$ARCH signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] \
http://packages.osrfoundation.org/gazebo/ubuntu-stable $DISTRO main"
echo "$REPO" | sudo tee /etc/apt/sources.list.d/gazebo-stable.list >/dev/null

echo
echo -e "\033[96m(4/4) ------------     Install ROS-Gazebo    ---------------\033[0m"
DIST=jazzy

echo -e "\033[34mInstalling ROS Gazebo framework...\033[0m"
sudo apt update && apt install -y \
    python3-rosdep \
    python3-rosinstall-generator \
    python3-vcstool \
    ros-$DIST-desktop \
    ros-$DIST-ros-gz \
    ros-$DIST-gz-ros2-control \
    ros-$DIST-effort-controllers \
    ros-$DIST-geographic-info \
    ros-$DIST-joint-state-publisher \
    ros-$DIST-joy-teleop \
    ros-$DIST-key-teleop \
    ros-$DIST-moveit-planners \
    ros-$DIST-moveit-simple-controller-manager \
    ros-$DIST-moveit-ros-visualization \
    ros-$DIST-robot-localization \
    ros-$DIST-ros2-controllers \
    ros-$DIST-teleop-tools \
    ros-$DIST-urdfdom-py \
    ros-dev-tools

echo
echo -e "\033[34mSetting up Gazebo source directory...\033[0m"
mkdir -p /opt/gazebo/src && cd /opt/gazebo/src || exit
curl -O https://raw.githubusercontent.com/gazebo-tooling/gazebodistro/master/collection-harmonic.yaml
vcs import < collection-harmonic.yaml

echo -e "\033[34mUpdating package list and installing dependencies...\033[0m"
# shellcheck disable=SC2046,SC2006
sudo apt -y install $(sort -u $(find . -iname 'packages-'`lsb_release -cs`'.apt' -o -iname 'packages.apt' | grep -v '/\.git/') | sed '/gz\|sdf/d' | tr '\n' ' ')

echo -e "\033[34mBuilding the project with colcon...\033[0m"
cd /opt/gazebo || exit
# Build gz-physics with limited cores to avoid memory issues
MAKEFLAGS="-j2" colcon build --cmake-args -DBUILD_TESTING=OFF --merge-install --packages-up-to gz-physics
# Full build
colcon build --cmake-args -DBUILD_TESTING=OFF --merge-install

echo
echo -e "\033[32m============================================================\033[0m"
echo -e "\033[32mROS-Gazebo Framework Installation completed. Awesome! 🤘🚀 \033[0m"
echo -e "Following command will set-up ROS-Gazebo environment variables to run it"
echo -e "\033[95msource /opt/ros/jazzy/setup.bash\033[0m"
echo -e "\033[95msource /opt/gazebo/install/setup.bash\033[0m"
echo -e "\033[95mexport PYTHONPATH=\$PYTHONPATH:/opt/gazebo/install/lib/python\033[0m"
echo -e "You may check ROS, and Gazebo version installed with \033[33mprintenv ROS_DISTRO\033[0m and \033[33mecho \$GZ_VERSION\033[0m"
echo
