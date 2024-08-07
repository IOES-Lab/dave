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
    python3-dbg python3-pip python3-venv \
    qtbase5-dev ruby dirmngr gnupg2 nano xauth \
    software-properties-common htop libtool \
    x11-apps mesa-utils bison flex automake && \
    rm -rf /var/lib/apt/lists/

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

# Set up Dave workspace
ENV DAVE_WS=/opt/ws_dave
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
    '$i source "/opt/ws_dave/install/setup.bash"' /ros_entrypoint.sh

# Source ROS and Gazebo
RUN sed --in-place --expression \
'$i source "/opt/ros/jazzy/setup.bash"' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i source "/opt/gazebo/install/setup.bash"' /ros_entrypoint.sh && \
sed --in-place --expression \
'$i export PYTHONPATH=$PYTHONPATH:/opt/gazebo/install/lib/python' /ros_entrypoint.sh

# Set User as user
USER $USER
RUN echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc  && \
    echo "source /opt/gazebo/install/setup.bash" >> ~/.bashrc
