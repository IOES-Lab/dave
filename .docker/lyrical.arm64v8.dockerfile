# DAVE development image for ROS 2 Lyrical / Gazebo Jetty on ARM64.
# Validated on Apple Silicon with Ubuntu 26.04, XFCE, and xrdp.
#
# Design notes:
# - FROM arm64v8/ubuntu:26.04 directly instead of a prebuilt personal base image — the RDP/xrdp
#   setup below is the adapted version of what that base image contained, plus the xrdp
#   group-permission fix required by the verified Apple Silicon build.
# - ROS_DISTRO=lyrical; ros-lyrical-ros-gz supplies the Gazebo Jetty vendor packages.
# - The current CUDA sonar remains disabled on ARM64; the open WGPU PR is intentionally excluded.
# - ArduSub SITL includes the Python 3.14 compatibility shims validated on Ubuntu 26.04
#   (imp/pipes modules, python-argparse removal, and PEP 668 handling).

FROM arm64v8/ubuntu:26.04

ARG USER=docker
ARG PASS=docker
ARG X11Forwarding=true

# --- RDP / XFCE desktop setup ---
# hadolint ignore=DL3008,DL3015,DL3009
RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
        apt-get install -y --no-install-recommends \
          dbus dbus-x11 xrdp xorgxrdp \
          xfce4 xfce4-goodies xfce4-terminal xterm \
          sudo openssl; \
    [ $X11Forwarding = 'true' ] && apt-get install -y openssh-server; \
    apt-get autoremove --purge; \
    apt-get clean; \
    rm -f /run/reboot-required*

RUN useradd -s /bin/bash -m $USER && echo "$USER:$PASS" | chpasswd; \
    usermod -aG sudo $USER; echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers; \
    adduser xrdp ssl-cert; \
    # xrdp daemon needs root-group access to session sockets under /run/xrdp/sockdir/ —
    # without this, RDP connections time out after ~30s ("connection problem, giving up")
    usermod -aG root xrdp; \
    echo 'LANG=en_US.UTF-8' >> /etc/default/locale; \
    echo 'export XDG_CURRENT_DESKTOP=XFCE' > /home/$USER/.xsessionrc; \
    echo 'export XDG_SESSION_DESKTOP=xfce' >> /home/$USER/.xsessionrc; \
    echo 'export DESKTOP_SESSION=xfce' >> /home/$USER/.xsessionrc; \
    echo 'export XDG_SESSION_TYPE=x11' >> /home/$USER/.xsessionrc; \
    sed -i "s/#EnableConsole=false/EnableConsole=true/g" /etc/xrdp/xrdp.ini; \
    sed -i 's/max_bpp=32/max_bpp=16/g' /etc/xrdp/xrdp.ini; \
    [ $X11Forwarding = 'true' ] && \
        sed -i 's/#X11UseLocalhost yes/X11UseLocalhost no/g' /etc/ssh/sshd_config || \
        sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/g' /etc/ssh/sshd_config || \
        :; \
    chown $USER:$USER /home/$USER/.xsessionrc

# Bypass the Debian Xsession wrapper. In a systemd-less container it can exit
# before reaching ~/.xsession. Each RDP login gets its own DBus session.
RUN printf '%s\n' \
      '#!/bin/bash' \
      'exec >>/home/docker/xrdp-startwm.log 2>&1' \
      'export HOME=/home/docker' \
      'export USER=docker' \
      'export LOGNAME=docker' \
      'export XDG_SESSION_TYPE=x11' \
      'export XDG_CURRENT_DESKTOP=XFCE' \
      'export XDG_SESSION_DESKTOP=xfce' \
      'export DESKTOP_SESSION=xfce' \
      'export XDG_RUNTIME_DIR="/run/user/$(id -u)"' \
      'unset GNOME_SHELL_SESSION_MODE SESSION_MANAGER DBUS_SESSION_BUS_ADDRESS' \
      'exec /usr/bin/dbus-run-session -- /usr/bin/startxfce4' \
      > /etc/xrdp/startwm.sh && \
    chmod 0755 /etc/xrdp/startwm.sh

ENV DEBIAN_FRONTEND=noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN=true
# hadolint ignore=DL3008
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    xterm vim net-tools \
    curl wget git build-essential cmake cppcheck \
    gnupg libeigen3-dev libgles2-mesa-dev \
    libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev \
    lsb-release pkg-config protobuf-compiler \
    python3-pip python3-venv \
    nano xauth htop libtool \
    x11-apps mesa-utils bison flex automake && \
    rm -rf /var/lib/apt/lists/

RUN truncate -s0 /tmp/preseed.cfg && \
    (echo "tzdata tzdata/Areas select Etc" >> /tmp/preseed.cfg) && \
    (echo "tzdata tzdata/Zones/Etc select UTC" >> /tmp/preseed.cfg) && \
    debconf-set-selections /tmp/preseed.cfg && \
    rm -f /etc/timezone && \
    dpkg-reconfigure -f noninteractive tzdata
RUN apt-get update && \
    apt-get -y install --no-install-recommends locales tzdata && \
    rm -rf /tmp/*
RUN locale-gen en_US en_US.UTF-8 && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8

EXPOSE 3389/tcp
EXPOSE 22/tcp

# --- ROS 2 Lyrical + Gazebo Jetty ---
ARG ROS_DISTRO="lyrical"
ENV GZ_VERSION=jetty

RUN apt update && apt full-upgrade -y && apt autoremove -y

# Gazebo Jetty is vendored by ros-lyrical-ros-gz on apt already — no separate Gazebo source build
RUN export ROS_APT_SOURCE_VERSION=$(curl -s https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest | grep -F "tag_name" | awk -F\" '{print $4}') && \
    curl -L -o /tmp/ros2-apt-source.deb \
      "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ROS_APT_SOURCE_VERSION}/ros2-apt-source_${ROS_APT_SOURCE_VERSION}.$(. /etc/os-release && echo ${UBUNTU_CODENAME:-${VERSION_CODENAME}})_all.deb" && \
    dpkg -i /tmp/ros2-apt-source.deb && \
    apt update && \
    apt install -y --no-install-recommends \
      ros-${ROS_DISTRO}-desktop ros-${ROS_DISTRO}-ros-gz \
      ros-${ROS_DISTRO}-image-view \
      python3-rosdep python3-vcstool python3-colcon-common-extensions

# --- DAVE workspace ---
# Build the exact checked-out revision supplied as the Docker build context.
ENV DAVE_UNDERLAY=/home/$USER/dave_ws
WORKDIR $DAVE_UNDERLAY/src
COPY . dave
RUN chown -R $USER:$USER $DAVE_UNDERLAY/src/dave

# Keep the companion repositories from the official DAVE workspace, while
# preserving the exact DAVE checkout copied above.
RUN vcs import --shallow --skip-existing \
      --input dave/extras/repos/dave.lyrical.repos && \
    chown -R $USER:$USER $DAVE_UNDERLAY

RUN rosdep init 2>/dev/null || true && rosdep update --rosdistro $ROS_DISTRO && \
    rosdep install --rosdistro $ROS_DISTRO -iy --from-paths .

USER $USER
WORKDIR $DAVE_UNDERLAY
RUN . "/opt/ros/${ROS_DISTRO}/setup.sh" && \
    colcon build --merge-install --executor sequential --symlink-install

# --- ArduSub SITL (BlueROV2) — Python 3.14 compatibility shims required, see notes/ardusub-sitl-setup.md ---
# Pinned to the commit used by the verified Ubuntu 26.04 / Python 3.14 build.
USER root
ARG ARDUSUB_COMMIT="30257f01185471ab4c1ac544e47d1b4437e44c98"
ARG ARDUPILOT_GAZEBO_COMMIT="082a0fe231f6e63bc8d1598f1cba461d9e2ea7f5"
WORKDIR /home/$USER
RUN git clone --recurse-submodules https://github.com/ArduPilot/ardupilot.git && \
    cd ardupilot && git fetch --tags && git checkout --detach "$ARDUSUB_COMMIT" && \
    git submodule update --init --recursive

RUN mkdir -p /home/$USER/imp_shim && \
    printf 'import types\ndef new_module(name):\n    return types.ModuleType(name)\n' > /home/$USER/imp_shim/imp.py && \
    printf 'import shlex\ndef quote(s):\n    return shlex.quote(s)\n' > /home/$USER/imp_shim/pipes.py

ENV PYTHONPATH=/home/$USER/imp_shim:/home/$USER/ardupilot/modules/waf/waflib/extras
ENV PIP_BREAK_SYSTEM_PACKAGES=1

# chown first — the repo was cloned as root but must be built as $USER, and git refuses to
# operate across ownership boundaries ("detected dubious ownership") since CVE-2022-24765
RUN chown -R $USER:$USER /home/$USER/ardupilot /home/$USER/imp_shim && \
    cd /home/$USER/ardupilot && \
    sed -i "s/ python-argparse//g" Tools/environment_install/install-prereqs-ubuntu.sh && \
    sed -i 's/-U pip setuptools wheel/-U pip "setuptools<80" wheel/' \
      Tools/environment_install/install-prereqs-ubuntu.sh && \
    su $USER -c "git config --global --add safe.directory /home/$USER/ardupilot" && \
    su $USER -c "cd /home/$USER/ardupilot && Tools/environment_install/install-prereqs-ubuntu.sh -y" && \
    su $USER -c "cd /home/$USER/ardupilot && python3 modules/waf/waf-light configure --board sitl" && \
    su $USER -c "cd /home/$USER/ardupilot && python3 modules/waf/waf-light build --target bin/ardusub" && \
    cp /home/$USER/ardupilot/build/sitl/bin/ardusub /usr/local/bin/ardusub

# Build the Gazebo Jetty ArduPilot plugin against the ROS vendor packages.
RUN git clone --depth 1 https://github.com/ArduPilot/ardupilot_gazebo.git \
      /home/$USER/ardupilot_gazebo && \
    cd /home/$USER/ardupilot_gazebo && \
    git fetch --depth 1 origin "$ARDUPILOT_GAZEBO_COMMIT" && \
    git checkout --detach "$ARDUPILOT_GAZEBO_COMMIT" && \
    chown -R $USER:$USER /home/$USER/ardupilot_gazebo && \
    mkdir -p /home/$USER/ardupilot_gazebo/build && \
    cd /home/$USER/ardupilot_gazebo/build && \
    . "/opt/ros/${ROS_DISTRO}/setup.sh" && \
    cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && \
    make -j2

ENV GZ_SIM_SYSTEM_PLUGIN_PATH=/home/$USER/ardupilot_gazebo/build
ENV GZ_SIM_RESOURCE_PATH=/home/$USER/ardupilot_gazebo/models:/home/$USER/ardupilot_gazebo/worlds

# --- QGroundControl, Firefox, environment ---
RUN usermod -aG dialout $USER && apt -y remove modemmanager || true
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    ffmpeg python3-venv python3-websockets \
    ros-${ROS_DISTRO}-joy-linux ros-${ROS_DISTRO}-mavros ros-${ROS_DISTRO}-mavros-msgs \
    gstreamer1.0-tools gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly python3-gi python3-gst-1.0 \
    libfuse2 libxcb-xinerama0 libxkbcommon-x11-0 libxcb-cursor-dev gstreamer1.0-qt6 \
    gstreamer1.0-gl libqt6qml6 qml6-module-qtquick qml6-module-qtquick-window && \
    /opt/ros/${ROS_DISTRO}/lib/mavros/install_geographiclib_datasets.sh && \
    test -r /usr/share/GeographicLib/geoids/egm96-5.pgm && \
    rm -rf /var/lib/apt/lists/*

USER $USER
RUN mkdir -p ~/QGC && wget -O ~/QGC/QGroundControl-aarch64-DailyBuild.AppImage \
    "https://d176tv9ibo4jno.cloudfront.net/builds/master/QGroundControl-aarch64.AppImage" && \
    cd ~/QGC && chmod +x QGroundControl-aarch64-DailyBuild.AppImage && \
    ./QGroundControl-aarch64-DailyBuild.AppImage --appimage-extract && \
    mv ~/QGC/squashfs-root/* ~/QGC/. && rm ~/QGC/QGroundControl-aarch64-DailyBuild.AppImage && \
    mkdir -p /home/$USER/.local/bin && \
    ln -sf /home/$USER/QGC/AppRun /home/$USER/.local/bin/qgroundcontrol

# Install Firefox from Mozilla's ARM64 archive, matching the previous image.
RUN curl -L "https://download.mozilla.org/?product=firefox-latest-ssl&os=linux64-aarch64&lang=en-US" \
      -o /tmp/firefox.tar.xz && \
    tar -xJf /tmp/firefox.tar.xz -C /home/$USER && \
    ln -sf /home/$USER/firefox/firefox /home/$USER/.local/bin/firefox && \
    rm -f /tmp/firefox.tar.xz

USER root
RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /home/$USER/.bashrc && \
    echo "source $DAVE_UNDERLAY/install/setup.bash" >> /home/$USER/.bashrc && \
    echo "export PATH=/home/$USER/.local/bin:/usr/local/bin:\$PATH" >> /home/$USER/.bashrc && \
    echo "export GZ_SIM_SYSTEM_PLUGIN_PATH=/home/$USER/ardupilot_gazebo/build:\${GZ_SIM_SYSTEM_PLUGIN_PATH:-}" >> /home/$USER/.bashrc && \
    echo "export GZ_SIM_RESOURCE_PATH=/home/$USER/ardupilot_gazebo/models:/home/$USER/ardupilot_gazebo/worlds:\${GZ_SIM_RESOURCE_PATH:-}" >> /home/$USER/.bashrc && \
    echo "export GEOGRAPHICLIB_GEOID_PATH=/usr/share/GeographicLib/geoids" >> /home/$USER/.bashrc && \
    echo "export GST_PLUGIN_PATH=/usr/lib/aarch64-linux-gnu/gstreamer-1.0:\${GST_PLUGIN_PATH:-}" >> /home/$USER/.bashrc && \
    echo "export QML2_IMPORT_PATH=/usr/lib/aarch64-linux-gnu/qt6/qml:\${QML2_IMPORT_PATH:-}" >> /home/$USER/.bashrc && \
    echo "export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:\${LD_LIBRARY_PATH:-}" >> /home/$USER/.bashrc && \
    echo "export PS1='\[\e[1;36m\]\u@lyrical_docker\[\e[0m\]:\[\e[1;34m\]\w\[\e[0m\]\$ '" >> /home/$USER/.bashrc

# --- Run (no systemd inside this container) ---
COPY extras/docker-arm64-entrypoint.sh /usr/local/bin/dave-rdp-entrypoint
RUN chmod 0755 /usr/local/bin/dave-rdp-entrypoint
CMD ["/usr/local/bin/dave-rdp-entrypoint"]
