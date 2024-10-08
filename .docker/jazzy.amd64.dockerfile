ARG ROS_DISTRO="jazzy"
FROM osrf/ros:$ROS_DISTRO-desktop-full
ARG BRANCH="ros2"

# Install Utilities
# hadolint ignore=DL3008
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    sudo xterm init systemd snapd vim net-tools \
    curl wget git build-essential cmake cppcheck \
    gnupg libeigen3-dev libgles2-mesa-dev \
    lsb-release pkg-config protobuf-compiler \
    python3-dbg python3-pip python3-venv python3-pexpect \
    python-is-python3 python3-future python3-wxgtk4.0 \
    qtbase5-dev ruby dirmngr gnupg2 nano xauth \
    software-properties-common htop libtool \
    x11-apps mesa-utils bison flex automake \
    && rm -rf /var/lib/apt/lists/

# Prereqs for Ardupilot - Ardusub
ADD --chown=root:root --chmod=0644 https://raw.githubusercontent.com/osrf/osrf-rosdep/master/gz/00-gazebo.list /etc/ros/rosdep/sources.list.d/00-gazebo.list
RUN wget https://packages.osrfoundation.org/gazebo.gpg -O /usr/share/keyrings/pkgs-osrf-archive-keyring.gpg \
    && echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/pkgs-osrf-archive-keyring.gpg] http://packages.osrfoundation.org/gazebo/ubuntu-stable $(lsb_release -cs) main" |  tee /etc/apt/sources.list.d/gazebo-stable.list > /dev/null \
    && apt-get update && apt-get install -y --no-install-recommends \
    libgz-sim8-dev rapidjson-dev libopencv-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    gstreamer1.0-plugins-bad gstreamer1.0-libav gstreamer1.0-gl \
    && rm -rf /var/lib/apt/lists/

# Locale for UTF-8
RUN truncate -s0 /tmp/preseed.cfg && \
   (echo "tzdata tzdata/Areas select Etc" >> /tmp/preseed.cfg) && \
   (echo "tzdata tzdata/Zones/Etc select UTC" >> /tmp/preseed.cfg) && \
   debconf-set-selections /tmp/preseed.cfg && \
   rm -f /etc/timezone && \
   dpkg-reconfigure -f noninteractive tzdata
# hadolint ignore=DL3008
RUN apt-get update && \
    apt-get -y install --no-install-recommends locales tzdata \
    && rm -rf /tmp/*
RUN locale-gen en_US en_US.UTF-8 && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 && \
    export LANG=en_US.UTF-8

# Make user (assume host user has 1000:1000 permission)
ARG USER=docker
SHELL ["/bin/bash", "-o", "pipefail", "-c"]
RUN adduser --shell /bin/bash --disabled-password --gecos '' $USER \
    && echo "$USER:$USER" | chpasswd && adduser $USER sudo \
    && echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

# Install ROS-Gazebo framework
ADD https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/ros-jazzy-binary-gz-harmonic-source-install.sh install.sh
RUN bash install.sh

# Install Ardupilot - Ardusub
ADD https://raw.githubusercontent.com/IOES-Lab/dave/ardusub_install/\
extras/ardusub-ubuntu-install.sh install.sh
RUN bash install.sh
# Install mavros
ADD https://raw.githubusercontent.com/IOES-Lab/dave/ardusub_install/\
extras/mavros-ubuntu-install.sh install.sh
RUN bash install.sh

# Set up Dave workspace
ENV DAVE_WS=/opt/dave_ws
WORKDIR $DAVE_WS/src

ADD https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/repos/dave.$ROS_DISTRO.repos $DAVE_WS/dave.repos
RUN vcs import --shallow --input $DAVE_WS/dave.repos

# Install dave dependencies
RUN apt-get update && rosdep update && \
    rosdep install -iy --from-paths . && \
    rm -rf /var/lib/apt/lists/

# Compile Dave
WORKDIR $DAVE_WS
RUN . "/opt/ros/${ROS_DISTRO}/setup.sh" && \
    colcon build

# source entrypoint setup
RUN touch /ros_entrypoint.sh && sed --in-place --expression \
    '$i source "/opt/dave_ws/install/setup.bash"' /ros_entrypoint.sh \
    && sed --in-place --expression \
    '$i cd /root' /ros_entrypoint.sh

RUN cp /home/docker/.bashrc ~/.bashrc

# Source ROS and Gazebo
RUN sed --in-place --expression \
'$i source "/opt/ros/jazzy/setup.bash"' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i source "/opt/gazebo/install/setup.bash"' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i source "/opt/mavros/install/setup.bash"' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i export GEOGRAPHICLIB_GEOID_PATH=/usr/local/share/GeographicLib/geoids' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i export PYTHONPATH=\$PYTHONPATH:/opt/gazebo/install/lib/python' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i export PATH=/opt/ardupilot_dave/ardupilot/build/sitl/bin:\$PATH' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i export PATH=/opt/ardupilot_dave/ardupilot/Tools/autotest:\$PATH' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i export GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/ardupilot_dave/ardupilot_gazebo/build:\$GZ_SIM_SYSTEM_PLUGIN_PATH' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i export GZ_SIM_RESOURCE_PATH=/opt/ardupilot_dave/ardupilot_gazebo/models:/opt/ardupilot_dave/ardupilot_gazebo/worlds:\$GZ_SIM_RESOURCE_PATH' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''\\033[1;37m \n=====\n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''  ____    ___     _______      _                     _   _      \n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\'' |  _ \\  / \\ \\   / | ____|    / \\   __ _ _   _  __ _| |_(_) ___ \n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\'' | | | |/ _ \\ \\ / /|  _|     / _ \\ / _` | | | |/ _` | __| |/ __|\n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\'' | |_| / ___ \\ V / | |___   / ___ \\ (_| | |_| | (_| | |_| | (__ \n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\'' |____/_/   \\_\\_/  |_____| /_/   \\_\\__, |\\__,_|\\__,_|\\__|_|\\___|\n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\'' __     ___      _               _     _____            _       \n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\'' \\ \\   / (_)_ __| |_ _   _  __ _| |   | ____|_ ____   _(_)_ __  \n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''  \\ \\ / /| | `__| __| | | |/ _` | |   |  _| | `_ \\ \\ / | | `__| \n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''   \\ V / | | |  | |_| |_| | (_| | |   | |___| | | \\ V /| | |_   \n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''    \\_/  |_|_|   \\__|\\__,_|\\__,_|_|   |_____|_| |_|\\_/ |_|_(_)  \n\\033[0m'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''\\033[1;32m\n =====\\033[0m\n'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''\\033[1;32m 👋 Hi! This is Docker virtual environment for DAVE\n\\033[0m'\'' ' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i printf '\''\\033[1;33m\tROS2 Jazzy - Gazebo Harmonic (w ardupilot(ardusub) + mavros)\n\n\\033[0m'\'' ' /ros_entrypoint.sh