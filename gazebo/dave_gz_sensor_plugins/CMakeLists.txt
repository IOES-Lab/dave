cmake_minimum_required(VERSION 3.8)
project(dave_gz_sensor_plugins)

# Find required packages
find_package(ament_cmake REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(gz-cmake 5 REQUIRED)
find_package(gz-plugin 4 REQUIRED COMPONENTS register)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(gz-common 7 REQUIRED COMPONENTS profiler)
find_package(gz-sim 10 REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(dave_interfaces REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gz-msgs 12 REQUIRED)
find_package(OpenCV REQUIRED)

# Set version variables
set(GZ_PLUGIN_VER "")
set(GZ_COMMON_VER "")
set(GZ_SIM_VER "")
set(GZ_MSGS_VER "")

include_directories(${OpenCV_INCLUDE_DIRS})

message(STATUS "Compiling against Gazebo Jetty")

add_library(UsblTransceiver SHARED src/UsblTransceiver.cc)
add_library(UsblTransponder SHARED src/UsblTransponder.cc)
add_library(sea_pressure_sensor SHARED src/sea_pressure_sensor.cc)
add_library(UnderwaterCamera SHARED src/UnderwaterCamera.cc)

target_include_directories(UsblTransceiver PRIVATE include)
target_include_directories(UsblTransponder PRIVATE include)
target_include_directories(sea_pressure_sensor PRIVATE include)
target_include_directories(UnderwaterCamera PRIVATE include)

target_link_libraries(UsblTransceiver
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

target_link_libraries(UsblTransponder
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

target_link_libraries(sea_pressure_sensor
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

target_link_libraries(UnderwaterCamera
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
  ${OpenCV_LIBS}
)

# Specify dependencies for FullSystem using ament_target_dependencies
target_link_libraries(UsblTransceiver
  ${dave_interfaces_TARGETS}
  ${sensor_msgs_TARGETS}
  rclcpp::rclcpp
  ${geometry_msgs_TARGETS}
  ${std_msgs_TARGETS}
)

target_link_libraries(UsblTransponder
  ${dave_interfaces_TARGETS}
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
)

target_link_libraries(sea_pressure_sensor
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  ${sensor_msgs_TARGETS}
  ${geometry_msgs_TARGETS}
)

target_link_libraries(UnderwaterCamera
  rclcpp::rclcpp
  ${sensor_msgs_TARGETS}
)

# Install targets
install(
  TARGETS
    UsblTransceiver
    UsblTransponder
    sea_pressure_sensor
    UnderwaterCamera
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(DIRECTORY include/
  DESTINATION include/
)

# Environment hooks
ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

# Testing setup
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
