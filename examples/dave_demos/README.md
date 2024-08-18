## Examples

### 1. Launching a Dave Object Model using Fuel URI

To launch a Dave model directly from a Fuel URI, follow these steps:

1. Build and source the workspace:

   ```bash
   colcon build && source install/setup.bash
   ```

2. Launch the model using the specified launch file:

   ```bash
   ros2 launch dave_demos dave_object.launch.py namespace:='mossy_cinder_block' paused:=false
   ```

This method simplifies the process by pulling the model directly from Fuel, ensuring you always have the latest version without needing to manage local files.

### 2. Launching a Dave Sensor Model using Downloaded Model Files

If you prefer to use model files downloaded from Fuel, proceed as follows:

1. Add a hook within the `dave_sensor_models` package to configure the necessary environment variables for Gazebo model lookup.

   ```bash
   cd <path-to-dave_ws>/src/dave/models/dave_sensor_models
   mkdir hooks && cd hooks
   touch dave_sensor_models.dsv.in
   echo "prepend-non-duplicate;GZ_SIM_RESOURCE_PATH;@CMAKE_INSTALL_PREFIX@/share/@PROJECT_NAME@" >> dave_sensor_models.dsv.in
   ```

2. Append the following line to the CMakeLists.txt file in the `dave_sensor_models` package:

   ```bash
   ament_environment_hooks("${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")
   ```

3. Build and source the workspace:

   ```bash
   cd <path-to-dave_ws>
   colcon build && source install/setup.bash
   ```

4. Launch the model using the provided launch file:

   ```bash
   ros2 launch dave_demos dave_sensor.launch.py namespace:='nortek_dvl500_300' world_name:=dvl_world paused:=false
   ```

This approach gives you more control over the models you use, allowing for offline use and customization. It's especially useful when working in environments with limited internet connectivity or when specific model versions are required.

### 3. Launching a World File

To launch a specific world file, you can specify the world name without the `.world` extension. Follow these steps:

1. Build and source the workspace:

```bash
colcon build && source install/setup.bash
```

1. Launch the world using the specified launch file

```bash
ros2 launch dave_demos dave_world.launch.py world_name:='dave_ocean_waves'
```

To check which worlds are available to launch, refer to `models/dave_worlds/worlds` directory.

The worlds files are linked to use models at https://app.gazebosim.org/ which means you need an internet connection to download the models and it takes some time to download at first launch. The files are saved in temporary directories and are reused in subsequent launches.

In this setup, you can dynamically specify different world files by changing the `world_name` argument in the launch command.
