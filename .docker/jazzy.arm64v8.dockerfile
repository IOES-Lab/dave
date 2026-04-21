# FROM arm64v8/ubuntu:24.04
# # Set RDP and SSH environments
# ARG X11Forwarding=true
# # hadolint ignore=DL3008,DL3015,DL3009
# RUN DEBIAN_FRONTEND=noninteractive apt-get update && \
#         apt-get install -y ubuntu-desktop-minimal dbus-x11 xrdp sudo; \
#     [ $X11Forwarding = 'true' ] && apt-get install -y openssh-server; \
#     apt-get autoremove --purge; \
#     apt-get clean; \
#     rm /run/reboot-required*

# ARG USER=docker
# ARG PASS=docker

# RUN useradd -s /bin/bash -m $USER -p "$(openssl passwd "$PASS")"; \
#     usermod -aG sudo $USER; echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers; \
#     adduser xrdp ssl-cert; \
#     # Setting the required environment variables
#     echo 'LANG=en_US.UTF-8' >> /etc/default/locale; \
#     echo 'export GNOME_SHELL_SESSION_MODE=ubuntu' > /home/$USER/.xsessionrc; \
#     echo 'export XDG_CURRENT_DESKTOP=ubuntu:GNOME' >> /home/$USER/.xsessionrc; \
#     echo 'export XDG_SESSION_TYPE=x11' >> /home/$USER/.xsessionrc; \
#     # Enabling log to the stdout
#     sed -i "s/#EnableConsole=false/EnableConsole=true/g" /etc/xrdp/xrdp.ini; \
#     # Disabling system animations and reducing the
#     # image quality to improve the performance
#     sed -i 's/max_bpp=32/max_bpp=16/g' /etc/xrdp/xrdp.ini; \
#     gsettings set org.gnome.desktop.interface enable-animations true; \
#     # Listening on wildcard address for X forwarding
#     [ $X11Forwarding = 'true' ] && \
#         sed -i 's/#X11UseLocalhost yes/X11UseLocalhost no/g' /etc/ssh/sshd_config || \
#         sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/g' /etc/ssh/sshd_config || \
#         :;

# # Disable initial welcome window
# RUN echo "X-GNOME-Autostart-enabled=false" >> /etc/xdg/autostart/gnome-initial-setup-first-login.desktop

# # Install basics
# ENV DEBIAN_FRONTEND noninteractive
# ENV DEBCONF_NONINTERACTIVE_SEEN=true
# # hadolint ignore=DL3008
# RUN apt-get update && \
#     apt-get install -y --no-install-recommends \
#     sudo xterm init systemd snapd vim net-tools \
#     curl wget git build-essential cmake cppcheck \
#     gnupg libeigen3-dev libgles2-mesa-dev \
#     lsb-release pkg-config protobuf-compiler \
#     python3-dbg python3-pip python3-venv \
#     qtbase5-dev ruby dirmngr gnupg2 nano xauth \
#     software-properties-common htop libtool \
#     x11-apps mesa-utils bison flex automake && \
#     rm -rf /var/lib/apt/lists/

# Locale for UTF-8
# RUN truncate -s0 /tmp/preseed.cfg && \
#    (echo "tzdata tzdata/Areas select Etc" >> /tmp/preseed.cfg) && \
#    (echo "tzdata tzdata/Zones/Etc select UTC" >> /tmp/preseed.cfg) && \
#    debconf-set-selections /tmp/preseed.cfg && \
#    rm -f /etc/timezone && \
#    dpkg-reconfigure -f noninteractive tzdata
# # hadolint ignore=DL3008
# RUN apt-get update && \
#     apt-get -y install --no-install-recommends locales tzdata \
#     && rm -rf /tmp/*
# RUN locale-gen en_US en_US.UTF-8 && update-locale LC_ALL=en_US.UTF-8 LANG=en_US.UTF-8 && \
#     export LANG=en_US.UTF-8

# # Run
# EXPOSE 3389/tcp
# EXPOSE 22/tcp
# # hadolint ignore=DL3025
# CMD sudo rm -f /var/run/xrdp/xrdp*.pid >/dev/null 2>&1; \
#     sudo service dbus restart >/dev/null 2>&1; \
#     sudo /usr/lib/systemd/systemd-logind >/dev/null 2>&1 & \
#     [ -f /usr/sbin/sshd ] && sudo /usr/sbin/sshd; \
#     sudo xrdp-sesman --config /etc/xrdp/sesman.ini; \
#     sudo xrdp --nodaemon --config /etc/xrdp/xrdp.ini

# Using the pre-built image for above commented out dockerfile code lines
# hadolint ignore=DL3007
FROM woensugchoi/ubuntu-arm-rdp-base:latest
ARG USER=docker

# ROS-Gazebo arg
ARG BRANCH="ros2"
ARG ROS_DISTRO="jazzy"

# Update OS
RUN apt update && apt full-upgrade -y && apt autoremove -y

# Install ROS-Gazebo framework (including ardusub and mavros)
ADD https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/ros-jazzy-gz-harmonic-install.sh install.sh
RUN sudo bash install.sh

# Download the background image from GitHub raw content URL
# hadolint ignore=DL3047
RUN wget -O /usr/share/backgrounds/custom-background.png -q \
    https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/background.png && \
    mv /usr/share/backgrounds/warty-final-ubuntu.png \
        /usr/share/backgrounds/warty-final-ubuntu.png.bak && \
    mv /usr/share/backgrounds/custom-background.png \
        /usr/share/backgrounds/warty-final-ubuntu.png && \
    cp /usr/share/backgrounds/warty-final-ubuntu.png \
        /usr/share/backgrounds/ubuntu-wallpaper-d.png

# Install QGroundControl
RUN usermod -aG dialout "$(id -un)" && apt -y remove modemmanager
RUN apt-get -q update && \
    apt-get install -y --no-install-recommends \
    ffmpeg python3-venv python3-websockets \
    ros-${ROS_DISTRO}-joy-linux gstreamer1.0-tools gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good gstreamer1.0-plugins-ugly python3-gi python3-gst-1.0 \
    libfuse2 libxcb-xinerama0 libxkbcommon-x11-0 libxcb-cursor-dev gstreamer1.0-qt6 \
    gstreamer1.0-gl libqt6qml6 qml6-module-qtquick qml6-module-qtquick-window && \
     rm -rf /var/lib/apt/lists/*
USER docker
RUN mkdir ~/QGC && wget -O ~/QGC/QGroundControl-aarch64-DailyBuild.AppImage \
    "https://d176tv9ibo4jno.cloudfront.net/builds/master/QGroundControl-aarch64.AppImage" && \
    cd ~/QGC && chmod +x QGroundControl-aarch64-DailyBuild.AppImage && \
    cd ~/QGC && ./QGroundControl-aarch64-DailyBuild.AppImage --appimage-extract && \
    mv ~/QGC/squashfs-root/* ~/QGC/. && rm ~/QGC/QGroundControl-aarch64-DailyBuild.AppImage && \
    mkdir -p /home/$USER/.local/bin && \
    ln -sf /home/$USER/QGC/AppRun /home/$USER/.local/bin/qgroundcontrol

# Install Firefox from Mozilla (aarch64 tarball)
RUN curl -L "https://download.mozilla.org/?product=firefox-latest-ssl&os=linux64-aarch64&lang=en-US" \
        -o /tmp/firefox.tar.xz && \
    tar -xJf /tmp/firefox.tar.xz -C /home/$USER && \
    ln -sf /home/$USER/firefox/firefox /home/$USER/.local/bin/firefox && \
    rm -f /tmp/firefox.tar.xz

# Set up Dave workspace
ENV DAVE_UNDERLAY=/home/$USER/dave_ws
WORKDIR $DAVE_UNDERLAY/src
RUN wget -O /home/$USER/dave_ws/dave.repos -q https://raw.githubusercontent.com/IOES-Lab/dave/$BRANCH/\
extras/repos/dave.$ROS_DISTRO.repos
RUN vcs import --shallow --input "/home/$USER/dave_ws/dave.repos"

# hadolint ignore=DL3027
RUN rosdep update --rosdistro $ROS_DISTRO && \
    rosdep install --rosdistro $ROS_DISTRO -iy --from-paths .

# Build dave workspace
WORKDIR $DAVE_UNDERLAY
RUN . "/opt/ros/${ROS_DISTRO}/setup.sh" && colcon build --symlink-install

# Set User as user
USER docker
RUN echo "source $DAVE_UNDERLAY/install/setup.bash" >> ~/.bashrc && \
    echo "source /opt/ros/jazzy/setup.bash" >> ~/.bashrc && \
    echo "export PATH=/opt/ardusub_ws/ardupilot/Tools/autotest:\$PATH" >> ~/.bashrc && \
    echo "export PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:\$PATH" >> ~/.bashrc && \
    echo "export GEOGRAPHICLIB_GEOID_PATH=/usr/share/GeographicLib/geoids" >> ~/.bashrc && \
    echo "export GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/ardusub_ws/ardupilot_gazebo/build:\$GZ_SIM_SYSTEM_PLUGIN_PATH" >> ~/.bashrc && \
    echo "export GZ_SIM_RESOURCE_PATH=/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds:\$GZ_SIM_RESOURCE_PATH" >> ~/.bashrc && \
    echo "export GST_PLUGIN_PATH=/usr/lib/aarch64-linux-gnu/gstreamer-1.0:\$GST_PLUGIN_PATH" >> ~/.bashrc && \
    echo "export QML2_IMPORT_PATH=/usr/lib/aarch64-linux-gnu/qt6/qml:\$QML2_IMPORT_PATH" >> ~/.bashrc && \
    echo "export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:\$LD_LIBRARY_PATH" >> ~/.bashrc && \
    echo "\n" >> ~/.bashrc && echo "if [ -d ~/HOST ]; then chown $USER:$USER ~/HOST; fi" >> ~/.bashrc  && \
    echo "export PS1='\[\e[1;36m\]\u@DAVE_docker\[\e[0m\]\[\e[1;34m\](\$(hostname | cut -c1-12))\[\e[0m\]:\[\e[1;34m\]\w\[\e[0m\]\$ '" >>  ~/.bashrc

# Other environment variables
RUN echo "export XDG_RUNTIME_DIR=~/.xdg_log" >> ~/.bashrc && \
    echo "unset SESSION_MANAGER" >> ~/.bashrc

# Create and write the welcome message to a new file
RUN mkdir -p /home/docker/.config/autostart && \
    printf '\033[1;36m =====\n' >> ~/.hi && \
    printf '  ____    ___     _______      _                     _   _      \n' >> ~/.hi && \
    printf ' |  _ \  / \ \   / | ____|    / \   __ _ _   _  __ _| |_(_) ___ \n' >> ~/.hi && \
    printf ' | | | |/ _ \ \ / /|  _|     / _ \ / _` | | | |/ _` | __| |/ __|\n' >> ~/.hi && \
    printf ' | |_| / ___ \ V / | |___   / ___ | (_| | |_| | (_| | |_| | (__ \n' >> ~/.hi && \
    printf ' |____/_/   \_\_/  |_____| /_/   \_\__, |\__,_|\__,_|\__|_|\___|\n' >> ~/.hi && \
    printf ' __     ___      _               _     _____            _       \n' >> ~/.hi && \
    printf ' \ \   / (_)_ __| |_ _   _  __ _| |   | ____|_ ____   _(_)_ __  \n' >> ~/.hi && \
    printf '  \ \ / /| | `__| __| | | |/ _` | |   |  _| | `_ \ \ / | | `__| \n' >> ~/.hi && \
    printf '   \ V / | | |  | |_| |_| | (_| | |   | |___| | | \ V /| | |_   \n' >> ~/.hi && \
    printf '    \_/  |_|_|   \__|\__,_|\__,_|_|   |_____|_| |_|\_/ |_|_(_)  \n\033[0m' >> ~/.hi && \
    printf '\033[1;32m\n =====\n\033[0m' >> ~/.hi && \
    printf "\\033[1;32m 👋 Hi! This is Docker virtual environment for DAVE\n\\033[0m" \
    >> ~/.hi && \
    printf "\\033[1;33m\tROS2 Jazzy - Gazebo Harmonic (w ardupilot(ardusub) + mavros)\n\n\n\\033[0m" \
    >> ~/.hi

# Remove sudo message
RUN touch /home/docker/.sudo_as_admin_successful

# Autostart terminal
# hadolint ignore=SC3037
RUN echo "[Desktop Entry]\nType=Application" \
    > /home/docker/.config/autostart/terminal.desktop && \
    echo "Exec=gnome-terminal -- bash -c 'cat ~/.hi; exec bash'" \
    >> /home/docker/.config/autostart/terminal.desktop && \
    echo -e "X-GNOME-Autostart-enabled=true" \
    >> /home/docker/.config/autostart/terminal.desktop
