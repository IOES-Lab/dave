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
find_package(dave_gz_world_plugins REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)

# Set version variables
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})

message(STATUS "Compiling against Gazebo Harmonic")

# Libraries
add_library(SphericalCoords SHARED src/SphericalCoords.cc)
add_library(OceanCurrentPlugin SHARED src/OceanCurrentPlugin.cc)

# Include directories
target_include_directories(SphericalCoords
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
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
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)
target_link_libraries(OceanCurrentPlugin
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

# Specify dependencies using ament_target_dependencies
ament_target_dependencies(SphericalCoords
  dave_interfaces
  rclcpp
  geometry_msgs
  std_msgs
)

ament_target_dependencies(OceanCurrentPlugin
  rclcpp
  std_msgs
  geometry_msgs
  dave_interfaces
  dave_gz_world_plugins
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
  gz-cmake3
  gz-plugin2
  gz-common5
  gz-sim8
  geometry_msgs
  std_msgs
  dave_interfaces
  dave_gz_world_plugins
  rclcpp
)

# Environment hooks
ament_environment_hooks("${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

# Optional: Testing setup
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

# Configure ament
ament_package()
