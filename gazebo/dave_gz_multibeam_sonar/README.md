# Dave Gazebo Multibeam Sonar Plugin

- The sensor itself is at `multibeam_sonar`
- The system plugin that will also be defined in `world` file is at `multibeam_sonar_system`
- We need both the sensor and the system plugin to be able to use the sensor in Gazebo

- Launch example with `dave_sensor.launch.py`:

  ```
  ros2 launch dave_demos dave_sensor.launch.py namespace:=blueview_p900 world_name:=dave_multibeam_sonar paused:=false x:=4 z:=0.5 yaw:=3.14
  ```

- Launch example with `ros_gz_bridge` to translate gazebo msg to ROS msg:
  - Not perfect

  ```
  ros2 launch dave_gz_multibeam_sonar multibeam_sonar_demo.launch.py
  ```