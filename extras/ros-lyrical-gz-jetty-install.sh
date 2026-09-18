#!/usr/bin/env bash
# Install the ROS 2 Lyrical / Gazebo Jetty development stack used by DAVE.
set -euo pipefail

DIST="${ROS_DISTRO:-lyrical}"
echo
echo -e "\033[94m============================================================\033[0m"
echo -e "\033[94m== One-liner Installation Script for ROS-Gazebo Framework ==\033[0m"
echo -e "\033[94m============================================================\033[0m"
echo -e "Target stack: ROS 2 Lyrical / Gazebo Jetty"
echo -e "Requirements: Ubuntu 26.04 LTS Resolute"
echo -e "\033[94m============================================================\033[0m"

echo
echo -e "\033[96m(1/5) -------------    Updating the System  ----------------\033[0m"
echo "Performing full system upgrade (this might take a while)..."
sudo apt-get update
sudo apt-get full-upgrade -y

echo
echo -e "\033[96m(2/5) ------------    Install Dependencies   ---------------\033[0m"
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
echo -e "\033[96m(3/5) ------------    Install Package Keys   ---------------\033[0m"
echo -e "\033[34mInstalling the ROS apt source...\033[0m"
# Remove keyring if exists to avoid conflicts
sudo rm -f /usr/share/keyrings/ros2-latest-archive-keyring.gpg && \
    sudo rm -rf /etc/apt/sources.list.d/ros2-latest.list
# Get Keys
sudo curl -sSL https://raw.githubusercontent.com/ros/rosdistro/master/ros.key \
    -o /usr/share/keyrings/ros-archive-keyring.gpg
sudo apt update && sudo apt install -y jq
# shellcheck disable=SC1091
. /etc/os-release
UBUNTU_CODENAME="${UBUNTU_CODENAME:-${VERSION_CODENAME}}"
ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | jq -r '.tag_name') && \
    curl -L -o /tmp/ros2-apt-source.deb \
    "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.${UBUNTU_CODENAME}_all.deb" && \
    sudo apt-get install -y /tmp/ros2-apt-source.deb && \
    rm -f /tmp/ros2-apt-source.deb

echo
echo -e "\033[96m(4/5) ------------     Install ROS-Gazebo    ---------------\033[0m"

echo -e "\033[34mInstalling ROS 2 ${DIST}; ros-${DIST}-ros-gz supplies Gazebo Jetty vendor packages...\033[0m"
sudo apt update && sudo apt install -y \
    python3-rosdep \
    python3-catkin-pkg \
    python3-rosinstall-generator \
    python3-colcon-core \
    python3-colcon-common-extensions \
    python3-vcstool \
    "ros-${DIST}-desktop-full" \
    "ros-${DIST}-ros-gz" \
    "ros-${DIST}-gz-ros2-control" \
    "ros-${DIST}-marine-acoustic-msgs" \
    ros-dev-tools

sudo rosdep init 2>/dev/null || true
rosdep update --rosdistro "$DIST"

echo
echo -e "\033[96m(5/5) ------------     Install ArduSub       ---------------\033[0m"
sudo apt update && sudo apt install -y \
    rapidjson-dev libopencv-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-bad gstreamer1.0-libav gstreamer1.0-gl \
    ffmpeg python3-venv python3-websockets \
    "ros-${DIST}-joy-linux" gstreamer1.0-tools gstreamer1.0-plugins-good gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-ugly python3-gi python3-gst-1.0 \
    libfuse2 libxcb-xinerama0 libxkbcommon-x11-0 libxcb-cursor-dev \
    cython3 python3-dev python3-lxml

# Install ArduSub. The helper sources the Lyrical environment so the
# ardupilot_gazebo build resolves Jetty's vendored CMake packages.
sudo mkdir -p /opt/ardusub_ws && cd /opt/ardusub_ws || exit
if [[ -n "${DAVE_EXTRAS_DIR:-}" && -f "${DAVE_EXTRAS_DIR}/ardusub-ubuntu-install.sh" ]]; then
    sudo cp "${DAVE_EXTRAS_DIR}/ardusub-ubuntu-install.sh" ./ardusub-ubuntu-install.sh
else
    sudo wget https://raw.githubusercontent.com/IOES-Lab/dave/ros2/extras/ardusub-ubuntu-install.sh
fi
sudo chmod +x ardusub-ubuntu-install.sh && sudo bash ./ardusub-ubuntu-install.sh

# Mavros install
sudo apt-get -y install "ros-${DIST}-mavros" "ros-${DIST}-mavros-msgs"
sudo mkdir -p /opt/mavros_ws && cd /opt/mavros_ws || exit
sudo wget https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh
sudo chmod +x install_geographiclib_datasets.sh && sudo bash ./install_geographiclib_datasets.sh

# Environment variables setup (write to ~/.dave/env and source from shell rc)
TARGET_USER="${SUDO_USER:-${USER:-}}"
if [[ -z "$TARGET_USER" ]]; then
    TARGET_USER="$(id -un)"
fi
TARGET_HOME="$(getent passwd "$TARGET_USER" | cut -d: -f6)"
TARGET_SHELL="$(getent passwd "$TARGET_USER" | cut -d: -f7)"

RC_FILE="$TARGET_HOME/.bashrc"
case "$TARGET_SHELL" in
    */zsh) RC_FILE="$TARGET_HOME/.zshrc" ;;
esac

ENV_DIR="$TARGET_HOME/.ros_ardusub_env"
ENV_FILE="$ENV_DIR/env"
mkdir -p "$ENV_DIR"

cat > "$ENV_FILE" <<EOF
if [ -n "\${ZSH_VERSION:-}" ]; then
    source /opt/ros/${DIST}/setup.zsh
else
    source /opt/ros/${DIST}/setup.bash
fi
export PATH=/opt/ardusub_ws/ardupilot/Tools/autotest:\$PATH
export PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:\$PATH
export GEOGRAPHICLIB_GEOID_PATH=/usr/share/GeographicLib/geoids
export GZ_VERSION=jetty
export GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/ardusub_ws/ardupilot_gazebo/build:\${GZ_SIM_SYSTEM_PLUGIN_PATH:-}
export GZ_SIM_RESOURCE_PATH=/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds:\${GZ_SIM_RESOURCE_PATH:-}
EOF

if ! grep -q "^source \$HOME/.ros_ardusub_env/env$" "$RC_FILE" 2>/dev/null; then
    echo "source \$HOME/.ros_ardusub_env/env" >> "$RC_FILE"
fi

chown -R "$TARGET_USER:$TARGET_USER" "$ENV_DIR"
chown "$TARGET_USER:$TARGET_USER" "$RC_FILE" 2>/dev/null || true

echo
echo -e "\033[32m============================================================\033[0m"
echo -e "\033[32mROS-Gazebo Framework (w mavros and Ardusub) Installation completed. Awesome! 🤘🚀 \033[0m"
echo -e "Check the installed stack with \033[33mprintenv ROS_DISTRO\033[0m and \033[33mgz sim --versions\033[0m"
echo
