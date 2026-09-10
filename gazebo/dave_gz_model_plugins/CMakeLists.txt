cmake_minimum_required(VERSION 3.8)
project(dave_gz_model_plugins)

# Find required packages
find_package(ament_cmake REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(gz-cmake 5 REQUIRED)
find_package(gz-plugin 4 REQUIRED COMPONENTS register)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(gz-common 7 REQUIRED COMPONENTS profiler)
find_package(gz-sim 10 REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(dave_interfaces REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gz-msgs 12 REQUIRED)
find_package(dave_gz_world_plugins REQUIRED)

# Set version variables
set(GZ_PLUGIN_VER "")
set(GZ_COMMON_VER "")
set(GZ_SIM_VER "")
set(GZ_MSGS_VER "")

message(STATUS "Compiling against Gazebo Jetty")

add_library(OceanCurrentModelPlugin SHARED src/OceanCurrentModelPlugin.cc)

target_include_directories(OceanCurrentModelPlugin PRIVATE include)

target_link_libraries(OceanCurrentModelPlugin
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

# Specify dependencies for ocean_current_plugin using ament_target_dependencies
target_link_libraries(OceanCurrentModelPlugin
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  ${geometry_msgs_TARGETS}
  ${dave_interfaces_TARGETS}
  dave_gz_world_plugins::gauss_markov_process
  dave_gz_world_plugins::tidal_oscillation
  dave_gz_world_plugins::dave_gz_world_plugins-msgs
)

# Install targets
install(TARGETS OceanCurrentModelPlugin
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
