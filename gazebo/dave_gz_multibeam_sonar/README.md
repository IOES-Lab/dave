# Dave Gazebo Multibeam Sonar Plugin

- The sensor itself is at `multibeam_sonar`
- The system plugin that will also be defined in `world` file is at `multibeam_sonar_system`
- We need both the sensor and the system plugin to be able to use the sensor in Gazebo

- To test
  ```
  ros2 launch dave_demos dave_world.launch.py world_name:=dave_multibeam_sonar verbose:=true
  ```