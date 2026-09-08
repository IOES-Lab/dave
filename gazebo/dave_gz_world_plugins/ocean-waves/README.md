For ocean-waves, you must add environment variables to your launch file to set the wave parameters.

```
# ensure the system plugins are found
export GZ_SIM_SYSTEM_PLUGIN_PATH=\
$GZ_SIM_SYSTEM_PLUGIN_PATH:\
/home/docker/HOST/dave_ws/install/wave/lib

# ensure the gui plugin is found
export GZ_GUI_PLUGIN_PATH=\
$GZ_GUI_PLUGIN_PATH:\
/home/docker/HOST/dave_ws/install/wave/src/gui/plugins/waves_control/build

# For current ogre vendor package bug, this is required (July 7, 2025)
ln -s /opt/ros/jazzy/opt/gz_ogre_next_vendor/lib/libOgreNextMain.so.2.3.3 /opt/ros/jazzy/opt/gz_ogre_next_vendor/lib/libOgreNextMain.so.2.3.1
```