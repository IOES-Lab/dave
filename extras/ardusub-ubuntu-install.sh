#!/usr/bin/env bash
set -euo pipefail

# Source Ros and Gazebo
ROS_DISTRO="${ROS_DISTRO:-lyrical}"
set +u
# shellcheck disable=SC1090
source "/opt/ros/${ROS_DISTRO}/setup.bash"
set -u
export GZ_VERSION="${GZ_VERSION:-jetty}"

# Commit used by the verified Lyrical / Python 3.14 build.
ARDUPILOT_COMMIT="${ARDUPILOT_COMMIT:-30257f01185471ab4c1ac544e47d1b4437e44c98}"
ARDUPILOT_GAZEBO_COMMIT="${ARDUPILOT_GAZEBO_COMMIT:-082a0fe231f6e63bc8d1598f1cba461d9e2ea7f5}"
mkdir -p "/opt/ardusub_ws" && cd "/opt/ardusub_ws" || exit
git clone https://github.com/ArduPilot/ardupilot.git --recurse-submodules
cd "/opt/ardusub_ws/ardupilot" || exit
git checkout --detach "$ARDUPILOT_COMMIT"
git submodule update --init --recursive

# ArduPilot's waf extras still import Python modules removed in 3.12/3.13.
# These minimal compatibility shims were used in the verified Ubuntu 26.04 build.
mkdir -p /opt/ardusub_ws/python_compat
printf 'import types\ndef new_module(name):\n    return types.ModuleType(name)\n' \
  > /opt/ardusub_ws/python_compat/imp.py
printf 'import shlex\ndef quote(value):\n    return shlex.quote(value)\n' \
  > /opt/ardusub_ws/python_compat/pipes.py
export PYTHONPATH="/opt/ardusub_ws/python_compat:/opt/ardusub_ws/ardupilot/modules/waf/waflib/extras${PYTHONPATH:+:$PYTHONPATH}"
export PIP_BREAK_SYSTEM_PACKAGES=1

# Install ArduSub dependencies
export SKIP_AP_EXT_ENV=1 SKIP_AP_GRAPHIC_ENV=1 SKIP_AP_COV_ENV=1 SKIP_AP_GIT_CHECK=1
# Do not install the STM development tools
export DO_AP_STM_ENV=0
# Do not activate the Ardupilot venv by default
export DO_PYTHON_VENV_ENV=0
sed -i 's/ python-argparse//g' Tools/environment_install/install-prereqs-ubuntu.sh
# Keep the ROS 2 Lyrical colcon requirement (setuptools < 80) intact.
sed -i 's/-U pip setuptools wheel/-U pip "setuptools<80" wheel/' \
  Tools/environment_install/install-prereqs-ubuntu.sh
# This system-wide helper is invoked by the root-owned Docker build. The pinned
# ArduPilot prerequisite script rejects EUID 0 before using sudo for the same
# package operations, so remove only that guard in this container installer.
# shellcheck disable=SC2016
sed -i '/^if \[ \$EUID == 0 \]; then$/,/^fi$/d' \
  Tools/environment_install/install-prereqs-ubuntu.sh
Tools/environment_install/install-prereqs-ubuntu.sh -y

# Build ArduSub
python3 modules/waf/waf-light configure --board sitl
python3 modules/waf/waf-light build --target bin/ardusub

# Clone ardupilot_gazebo code
cd "/opt/ardusub_ws" || exit
git clone https://github.com/ArduPilot/ardupilot_gazebo.git
cd "/opt/ardusub_ws/ardupilot_gazebo" || exit
git checkout --detach "$ARDUPILOT_GAZEBO_COMMIT"

# Install ardupilot_gazebo plugin
# Check if the directory creation was successful
mkdir -p "/opt/ardusub_ws/ardupilot_gazebo/build" \
  && cd "/opt/ardusub_ws/ardupilot_gazebo/build" || exit
cmake .. -DCMAKE_BUILD_TYPE=RelWithDebInfo && make -j2

# Add results of ArduSub build
export PATH=/opt/ardusub_ws/ardupilot/build/sitl/bin:$PATH
# Optional: add autotest to the PATH, helpful for running sim_vehicle.py
export PATH=/opt/ardusub_ws/ardupilot/Tools/autotest:$PATH
# Add ardupilot_gazebo plugin
export GZ_SIM_SYSTEM_PLUGIN_PATH=/opt/ardusub_ws/ardupilot_gazebo/build:${GZ_SIM_SYSTEM_PLUGIN_PATH:-}
# Add ardupilot_gazebo models and worlds
export GZ_SIM_RESOURCE_PATH=/opt/ardusub_ws/ardupilot_gazebo/models:/opt/ardusub_ws/ardupilot_gazebo/worlds:${GZ_SIM_RESOURCE_PATH:-}
