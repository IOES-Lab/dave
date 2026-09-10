ARG ROS_DISTRO="lyrical"
FROM osrf/ros:${ROS_DISTRO}-desktop-full

ARG ROS_DISTRO="lyrical"
ENV ROS_DISTRO=${ROS_DISTRO}
ENV DEBIAN_FRONTEND=noninteractive

# The installer expects sudo even when the image build runs as root.
RUN apt-get update && \
    apt-get install -y --no-install-recommends sudo ca-certificates xz-utils && \
    rm -rf /var/lib/apt/lists/*

# Install ROS 2 Lyrical, Gazebo Jetty, ArduSub, and MAVROS from this checkout.
COPY extras /tmp/dave-extras
RUN DAVE_EXTRAS_DIR=/tmp/dave-extras \
    bash /tmp/dave-extras/ros-lyrical-gz-jetty-install.sh

# Install QGroundControl.
RUN mkdir -p /opt/QGC && cd /opt/QGC && \
    wget -O QGroundControl-x86_64.AppImage \
      "https://d176tv9ibo4jno.cloudfront.net/latest/QGroundControl-x86_64.AppImage" && \
    chmod +x QGroundControl-x86_64.AppImage && \
    ./QGroundControl-x86_64.AppImage --appimage-extract && \
    mv squashfs-root/* /opt/QGC/ && \
    rm QGroundControl-x86_64.AppImage && \
    ln -sf /opt/QGC/AppRun /usr/local/bin/qgroundcontrol

# Install Firefox from Mozilla.
RUN curl -L "https://download.mozilla.org/?product=firefox-latest-ssl&os=linux64&lang=en-US" \
      -o /tmp/firefox.tar.xz && \
    tar -xJf /tmp/firefox.tar.xz -C /opt && \
    ln -sf /opt/firefox/firefox /usr/local/bin/firefox && \
    rm -f /tmp/firefox.tar.xz

# Build the exact DAVE revision supplied as the Docker build context. Import the
# companion repositories, but never replace the checked-out DAVE source.
ENV DAVE_WS=/opt/dave_ws
WORKDIR $DAVE_WS/src
COPY . dave
RUN vcs import --shallow --skip-existing \
      --input dave/extras/repos/dave.lyrical.repos

RUN rosdep update --rosdistro "$ROS_DISTRO" && \
    rosdep install --rosdistro "$ROS_DISTRO" -iy --from-paths . && \
    rm -rf /var/lib/apt/lists/*

WORKDIR $DAVE_WS
RUN . "/opt/ros/${ROS_DISTRO}/setup.sh" && \
    colcon build --merge-install --executor sequential --symlink-install

RUN echo "source /opt/ros/${ROS_DISTRO}/setup.bash" >> /root/.bashrc && \
    echo "source $DAVE_WS/install/setup.bash" >> /root/.bashrc && \
    echo "export PS1='\[\e[1;36m\]\u@DAVE_docker\[\e[0m\]\[\e[1;34m\](\$(hostname | cut -c1-12))\[\e[0m\]:\[\e[1;34m\]\w\[\e[0m\]\$ '" >> /root/.bashrc

RUN touch /root/.dave_entrypoint && \
    printf '\033[1;36mDAVE underwater robotics simulation\033[0m\n' >> /root/.dave_entrypoint && \
    printf '\033[1;33mROS 2 Lyrical · Gazebo Jetty · ArduSub · MAVROS\033[0m\n\n' >> /root/.dave_entrypoint && \
    echo 'cat /root/.dave_entrypoint' >> /root/.bashrc

WORKDIR /root
