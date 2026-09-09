#!/usr/bin/env bash
set -euo pipefail

# Source Ros and Gazebo
ROS_DISTRO="${ROS_DISTRO:-lyrical}"
set +u
# shellcheck disable=SC1090
source "/opt/ros/${ROS_DISTRO}/setup.bash"
set -u

USER_NAME=$(logname 2>/dev/null || echo "${SUDO_USER:-$USER}")
export USER="$USER_NAME"

ARDUPILOT_COMMIT="${ARDUPILOT_COMMIT:-30257f01185471ab4c1ac544e47d1b4437e44c98}"
mkdir -p "/home/$USER/ardusub_ws" && cd "/home/$USER/ardusub_ws" || exit
git clone https://github.com/ArduPilot/ardupilot.git --recurse-submodules
cd "/home/$USER/ardusub_ws/ardupilot" || exit
git checkout --detach "$ARDUPILOT_COMMIT"
git submodule update --init --recursive

mkdir -p "/home/$USER/ardusub_ws/python_compat"
printf 'import types\ndef new_module(name):\n    return types.ModuleType(name)\n' \
  > "/home/$USER/ardusub_ws/python_compat/imp.py"
printf 'import shlex\ndef quote(value):\n    return shlex.quote(value)\n' \
  > "/home/$USER/ardusub_ws/python_compat/pipes.py"
export PYTHONPATH="/home/$USER/ardusub_ws/python_compat:/home/$USER/ardusub_ws/ardupilot/modules/waf/waflib/extras${PYTHONPATH:+:$PYTHONPATH}"
export PIP_BREAK_SYSTEM_PACKAGES=1

# Install ArduSub dependencies
export SKIP_AP_EXT_ENV=1 SKIP_AP_GRAPHIC_ENV=1 SKIP_AP_COV_ENV=1 SKIP_AP_GIT_CHECK=1
# Do not install the STM development tools
export DO_AP_STM_ENV=0
# Do not activate the Ardupilot venv by default
export DO_PYTHON_VENV_ENV=0
sed -i 's/ python-argparse//g' Tools/environment_install/install-prereqs-ubuntu.sh
Tools/environment_install/install-prereqs-ubuntu.sh -y

# Build ArduSub
modules/waf/waf-light configure --board sitl \
  && modules/waf/waf-light build --target bin/ardusub

# Clone ardupilot_gazebo code
cd "/home/$USER/ardusub_ws" || exit
git clone https://github.com/ArduPilot/ardupilot_gazebo.git

# Install ardupilot_gazebo plugin
# Check if the directory creation was successful
mkdir -p "/home/$USER/ardusub_ws/ardupilot_gazebo/build" \
  && cd "/home/$USER/ardusub_ws/ardupilot_gazebo/build" || exit
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && make -j2

# Add results of ArduSub build
export PATH=/home/$USER/ardusub_ws/ardupilot/build/sitl/bin:$PATH
# Optional: add autotest to the PATH, helpful for running sim_vehicle.py
export PATH=/home/$USER/ardusub_ws/ardupilot/Tools/autotest:$PATH
# Add ardupilot_gazebo plugin
export GZ_SIM_SYSTEM_PLUGIN_PATH=/home/$USER/ardusub_ws/ardupilot_gazebo/build:${GZ_SIM_SYSTEM_PLUGIN_PATH:-}
# Add ardupilot_gazebo models and worlds
export GZ_SIM_RESOURCE_PATH=/home/$USER/ardusub_ws/ardupilot_gazebo/models:/home/$USER/ardusub_ws/ardupilot_gazebo/worlds:${GZ_SIM_RESOURCE_PATH:-}
