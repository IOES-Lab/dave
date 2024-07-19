cmake_minimum_required(VERSION 3.8)

#Define the project name
project(dave_gz_world_plugins)

find_package(ament_cmake REQUIRED)
find_package(ros_gz_example_description REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(gz-cmake3 REQUIRED)
find_package(gz-plugin2 REQUIRED COMPONENTS register)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(gz-common5 REQUIRED COMPONENTS profiler)
find_package(gz-sim8 REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(dave_interfaces REQUIRED)

# Set version variables with gazebo Harmonic
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})

message(STATUS "compiling against Gazebo Harmonic")

add_library(dave_gz_world_plugins SHARED
src/ocean_current_world_plugins.cpp)

target_include_directories(ocean_current_world_plugins PRIVATE include)

target_link_libraries(ocean_current_world_plugins
gz-sim${GZ_SIM_VER}::GZ-SIM${GZ_SIM_VER})

ament_target_dependencies(ocean_current_world_plugin
  "rclcpp"
  "std_msgs"
  "geometry_msgs"
#   "dave_interfaces"
)

# Install targets
install(TARGETS ocean_current_world_plugin
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(DIRECTORY include/
  DESTINATION include/
)

# Environment Hooks
ament_environment_hooks("${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

ament_package()