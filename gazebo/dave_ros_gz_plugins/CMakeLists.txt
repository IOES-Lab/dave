cmake_minimum_required(VERSION 3.8)
project(dave_ros_gz_plugins)

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
find_package(dave_gz_world_plugins REQUIRED)
find_package(Boost REQUIRED)

# Set version variables
set(GZ_PLUGIN_VER "")
set(GZ_COMMON_VER "")
set(GZ_SIM_VER "")

message(STATUS "Compiling against Gazebo Jetty")

# Libraries
add_library(SphericalCoords SHARED src/SphericalCoords.cc)
add_library(DVLBridge SHARED src/DVLBridge.cc)
add_library(OceanCurrentPlugin SHARED src/OceanCurrentPlugin.cc)

# Include directories
target_include_directories(SphericalCoords PRIVATE include)
target_include_directories(DVLBridge PRIVATE include)
target_include_directories(OceanCurrentPlugin
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    include ${dave_gz_world_plugins_INCLUDE_DIRS}
    include ${dave_interfaces_INCLUDE_DIRS}
)

# Linking
target_link_libraries(SphericalCoords
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER})
target_link_libraries(DVLBridge
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)
target_link_libraries(OceanCurrentPlugin
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

# Specify dependencies using ament_target_dependencies
target_link_libraries(SphericalCoords
  ${dave_interfaces_TARGETS}
  rclcpp::rclcpp
  ${geometry_msgs_TARGETS}
  ${std_msgs_TARGETS}
)
target_link_libraries(DVLBridge
  ${dave_interfaces_TARGETS}
  rclcpp::rclcpp
  ${geometry_msgs_TARGETS}
  ${std_msgs_TARGETS}
)
target_link_libraries(OceanCurrentPlugin
  rclcpp::rclcpp
  ${std_msgs_TARGETS}
  ${geometry_msgs_TARGETS}
  ${dave_interfaces_TARGETS}
  dave_gz_world_plugins::OceanCurrentWorldPlugin
  dave_gz_world_plugins::gauss_markov_process
  dave_gz_world_plugins::tidal_oscillation
  dave_gz_world_plugins::dave_gz_world_plugins-msgs
)

# Install targets
install(TARGETS SphericalCoords DVLBridge
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Install targets
install(
  TARGETS SphericalCoords OceanCurrentPlugin
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

ament_export_include_directories("include/${PROJECT_NAME}")
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(
  gz-cmake
  gz-plugin
  gz-common
  gz-sim
  geometry_msgs
  std_msgs
  dave_interfaces
  dave_gz_world_plugins
  rclcpp
)

# Environment hooks
ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

# Optional: Testing setup
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# Configure ament
ament_package()
