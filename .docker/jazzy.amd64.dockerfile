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

# Install ROS-Gazebo framework
ADD https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/ros-jazzy-gz-harmonic-install.sh install.sh
RUN bash install.sh

# Install Ardupilot - Ardusub
ADD https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/ardusub-ubuntu-install.sh install.sh
RUN bash install.sh
# Install mavros
RUN apt-get update && \
    apt-get -y install --no-install-recommends ros-jazzy-mavros* \
    && rm -rf /tmp/*
WORKDIR /opt/mavros_ws
RUN wget https://raw.githubusercontent.com/mavlink/mavros/master/mavros/scripts/install_geographiclib_datasets.sh && \
    bash ./install_geographiclib_datasets.sh

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
WORKDIR /

# Set up bashrc for root
RUN echo "source /opt/ros/jazzy/setup.bash" >> /root/.bashrc && \
    echo "source /opt/dave_ws/install/setup.bash" >> /root/.bashrc && \
    echo "export PATH=/opt/ardusub_ws/ardupilot/Tools/autotest:\$PATH" >> /root/.bashrc && \
    echo "export PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:\$PATH" >> /root/.bashrc && \
    echo "export GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/ardusub_ws/ardupilot_gazebo/build:\$GZ_SIM_SYSTEM_PLUGIN_PATH" >> /root/.bashrc && \
    echo "export GZ_SIM_RESOURCE_PATH=/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds:\$GZ_SIM_RESOURCE_PATH" >> /root/.bashrc && \
    echo "export PS1='\[\e[1;36m\]\u@DAVE_docker\[\e[0m\]\[\e[1;34m\](\$(hostname | cut -c1-12))\[\e[0m\]:\[\e[1;34m\]\w\[\e[0m\]\$ '" >> /root/.bashrc

RUN touch /root/.dave_entrypoint && printf '\033[1;36m =====\n' >> /root/.dave_entrypoint && \
    printf '  ____    ___     _______      _                     _   _      \n' >> /root/.dave_entrypoint && \
    printf ' |  _ \  / \ \   / | ____|    / \   __ _ _   _  __ _| |_(_) ___ \n' >> /root/.dave_entrypoint && \
    printf ' | | | |/ _ \ \ / /|  _|     / _ \ / _` | | | |/ _` | __| |/ __|\n' >> /root/.dave_entrypoint && \
    printf ' | |_| / ___ \ V / | |___   / ___ | (_| | |_| | (_| | |_| | (__ \n' >> /root/.dave_entrypoint && \
    printf ' |____/_/   \_\_/  |_____| /_/   \_\__, |\__,_|\__,_|\__|_|\___|\n' >> /root/.dave_entrypoint && \
    printf ' __     ___      _               _     _____            _       \n' >> /root/.dave_entrypoint && \
    printf ' \ \   / (_)_ __| |_ _   _  __ _| |   | ____|_ ____   _(_)_ __  \n' >> /root/.dave_entrypoint && \
    printf '  \ \ / /| | `__| __| | | |/ _` | |   |  _| | `_ \ \ / | | `__| \n' >> /root/.dave_entrypoint && \
    printf '   \ V / | | |  | |_| |_| | (_| | |   | |___| | | \ V /| | |_   \n' >> /root/.dave_entrypoint && \
    printf '    \_/  |_|_|   \__|\__,_|\__,_|_|   |_____|_| |_|\_/ |_|_(_)  \n\033[0m' >> /root/.dave_entrypoint && \
    printf '\033[1;32m\n =====\n\033[0m' >> /root/.dave_entrypoint && \
    printf "\\033[1;32m 👋 Hi! This is Docker virtual environment for DAVE\n\\033[0m" \
    >> /root/.dave_entrypoint && \
    printf "\\033[1;33m\tROS2 Jazzy - Gazebo Harmonic (w ardupilot(ardusub) + mavros)\n\n\\033[0m" \
    >> /root/.dave_entrypoint && \
    echo 'cat /root/.dave_entrypoint' >> /root/.bashrc

WORKDIR /root
