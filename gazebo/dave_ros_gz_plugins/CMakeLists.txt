cmake_minimum_required(VERSION 3.8)
project(dave_ros_gz_plugins)

# Find required packages
find_package(ament_cmake REQUIRED)
find_package(gz-cmake3 REQUIRED)
find_package(gz-plugin2 REQUIRED COMPONENTS register)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(gz-common5 REQUIRED COMPONENTS profiler)
find_package(gz-sim8 REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(dave_interfaces REQUIRED)

# Set version variables
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})

message(STATUS "Compiling against Gazebo Harmonic")

add_library(SphericalCoords SHARED src/SphericalCoords.cc)
add_library(DVLBridge SHARED src/DVLBridge.cc)

target_include_directories(SphericalCoords PRIVATE include)
target_include_directories(DVLBridge PRIVATE include)

target_link_libraries(SphericalCoords
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER})
target_link_libraries(DVLBridge
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER})

# Specify dependencies for FullSystem using ament_target_dependencies
ament_target_dependencies(SphericalCoords
  dave_interfaces
  rclcpp
  geometry_msgs
  std_msgs
)
ament_target_dependencies(DVLBridge
  dave_interfaces
  rclcpp
  geometry_msgs
  std_msgs
)

# Install targets
install(TARGETS SphericalCoords DVLBridge
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(DIRECTORY include/
  DESTINATION include/
)

# Environment hooks
ament_environment_hooks(
"${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

# Testing setup
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# Configure ament
ament_package()
