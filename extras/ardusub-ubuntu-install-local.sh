#!/bin/bash

# Source Ros and Gazebo
# shellcheck source=/dev/null
source /opt/ros/jazzy/setup.bash
# shellcheck source=/dev/null
source /opt/gazebo/install/setup.bash

export USER=docker

# Really should do version pinning but Sub-4.5 is waaaay behind master
# (e.g. it doesn't know about "noble" yet)
export ARDUPILOT_RELEASE=master
mkdir -p "/home/$USER/ardupilot_dave" && cd "/home/$USER/ardupilot_dave" || exit
git clone -b $ARDUPILOT_RELEASE https://github.com/ArduPilot/ardupilot.git --recurse-submodules

# Install ArduSub dependencies
cd "/home/$USER/ardupilot_dave/ardupilot" || exit
export SKIP_AP_EXT_ENV=1 SKIP_AP_GRAPHIC_ENV=1 SKIP_AP_COV_ENV=1 SKIP_AP_GIT_CHECK=1
# Do not install the STM development tools
export DO_AP_STM_ENV=0
# Do not activate the Ardupilot venv by default
export DO_PYTHON_VENV_ENV=0
Tools/environment_install/install-prereqs-ubuntu.sh -y

# Build ArduSub
modules/waf/waf-light configure --board sitl \
  && modules/waf/waf-light build --target bin/ardusub

# Clone ardupilot_gazebo code
cd "/home/$USER/ardupilot_dave" || exit
git clone https://github.com/ArduPilot/ardupilot_gazebo.git

# Install ardupilot_gazebo plugin
# Check if the directory creation was successful
mkdir -p "/home/$USER/ardupilot_dave/ardupilot_gazebo/build" \
  && cd "/home/$USER/ardupilot_dave/ardupilot_gazebo/build" || exit
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && make -j2

# Add results of ArduSub build
export PATH=/home/$USER/ardupilot_dave/ardupilot/build/sitl/bin:\$PATH
# Optional: add autotest to the PATH, helpful for running sim_vehicle.py
export PATH=/home/$USER/ardupilot_dave/ardupilot/Tools/autotest:\$PATH
# Add ardupilot_gazebo plugin
export GZ_SIM_SYSTEM_PLUGIN_PATH=/home/$USER/ardupilot_dave/ardupilot_gazebo/build:\$GZ_SIM_SYSTEM_PLUGIN_PATH
# Add ardupilot_gazebo models and worlds
export GZ_SIM_RESOURCE_PATH=/home/$USER/ardupilot_dave/ardupilot_gazebo/models:/home/$USER/ardupilot_dave/ardupilot_gazebo/worlds:\$GZ_SIM_RESOURCE_PATH