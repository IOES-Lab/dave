cmake_minimum_required(VERSION 3.8)

project(dave_gz_model_plugins)

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
find_package(dave_gz_world_plugins REQUIRED)

# Set version variables with gazebo Harmonic
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})
set(GZ_MSGS_VER ${gz-msgs10_VERSION_MAJOR})

message(STATUS "Compiling against Gazebo Harmonic")

add_library(ocean_current_model_plugin SHARED
  src/ocean_current_model_plugin.cc
)

target_include_directories(ocean_current_model_plugin
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

target_link_libraries(ocean_current_model_plugin
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER})


# Specify dependencies for ocean_current_plugin using ament_target_dependencies
ament_target_dependencies(ocean_current_model_plugin
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
  TARGETS ocean_current_model_plugin
  EXPORT export_${PROJECT_NAME}
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_dependencies(${THIS_PACKAGE_INCLUDE_DEPENDS})

# Environment hooks
ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

# Following directives are used when testing.
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()

