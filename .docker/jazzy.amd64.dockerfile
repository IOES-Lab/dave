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

# Install ROS-Gazebo framework (including ardusub and mavros)
ADD https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/ros-jazzy-gz-harmonic-install.sh install.sh
RUN bash install.sh

# Install QGroundControl
RUN mkdir -p /opt/QGC && cd /opt/QGC && wget -O /opt/QGC/QGroundControl-x86_64.AppImage \
    "https://d176tv9ibo4jno.cloudfront.net/latest/QGroundControl-x86_64.AppImage" && \
    chmod +x QGroundControl-x86_64.AppImage && \
    ./QGroundControl-x86_64.AppImage --appimage-extract && \
    mv squashfs-root/* /opt/QGC/ && rm QGroundControl-x86_64.AppImage && \
    ln -sf /opt/QGC/AppRun /usr/local/bin/qgroundcontrol

# Install Firefox from Mozilla (aarch64 tarball)
RUN curl -L "https://download.mozilla.org/?product=firefox-latest-ssl&os=linux64&lang=en-US" \
        -o /tmp/firefox.tar.xz && \
    mkdir -p /opt/firefox && \
    tar -xJf /tmp/firefox.tar.xz -C /opt && \
    ln -sf /opt/firefox/firefox /usr/local/bin/firefox && \
    rm -f /tmp/firefox.tar.xz

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
    colcon build --symlink-install
WORKDIR /

# Set up bashrc for root
RUN echo "source $DAVE_WS/install/setup.bash" >> /root/.bashrc && \
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
