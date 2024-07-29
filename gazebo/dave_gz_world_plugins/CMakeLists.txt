cmake_minimum_required(VERSION 3.8)

#Define the project name
project(dave_gz_world_plugins)

find_package(ament_cmake REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(gz-cmake3 REQUIRED)
find_package(gz-plugin2 REQUIRED COMPONENTS register)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(gz-common5 REQUIRED COMPONENTS profiler)
find_package(gz-sim8 REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(dave_interfaces REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gz-msgs10 REQUIRED)

# Set version variables with gazebo Harmonic
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})
set(GZ_MSGS_VER ${gz-msgs10_VERSION_MAJOR})

message(STATUS "compiling against Gazebo Harmonic")

add_library(ocean_current_world_plugin SHARED
src/ocean_current_world_plugin.cc)
add_library(gauss_markov_process SHARED
src/gauss_markov_process.cc)
add_library(tidal_oscillation SHARED
src/tidal_oscillation.cc)

target_include_directories(ocean_current_world_plugin PRIVATE include)
target_include_directories(gauss_markov_process PRIVATE include)
target_include_directories(tidal_oscillation PRIVATE include)

target_link_libraries(ocean_current_world_plugin
gz-sim${GZ_SIM_VER}::GZ-SIM${GZ_SIM_VER})
target_link_libraries(gauss_markov_process
gz-sim${GZ_SIM_VER}::GZ-SIM${GZ_SIM_VER})
target_link_libraries(tidal_oscillation
gz-sim${GZ_SIM_VER}::GZ-SIM${GZ_SIM_VER})

ament_target_dependencies(ocean_current_world_plugin
  "rclcpp"
  "std_msgs"
  "geometry_msgs"
  "dave_interfaces"
)

ament_target_dependencies(gauss_markov_process
  "rclcpp"
  "std_msgs"
  "geometry_msgs"
  "dave_interfaces"
)

ament_target_dependencies(tidal_oscillation
  "rclcpp"
  "std_msgs"
  "geometry_msgs"
  "dave_interfaces"
)

# Install targets
install(TARGETS ocean_current_world_plugin gauss_markov_process tidal_oscillation
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