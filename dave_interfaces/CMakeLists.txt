cmake_minimum_required(VERSION 3.8)
project(dave_interfaces)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/UsblCommand.msg"
  "msg/UsblResponse.msg"
  "msg/Location.msg"
  "msg/StratifiedCurrentVelocity.msg"
  "msg/StratifiedCurrentDatabase.msg"
  "srv/SetOriginSphericalCoord.srv"
  "srv/GetOriginSphericalCoord.srv"
  "srv/TransformToSphericalCoord.srv"
  "srv/TransformFromSphericalCoord.srv"
  "srv/GetCurrentModel.srv"
  "srv/SetCurrentModel.srv"
  "srv/SetCurrentDirection.srv"
  "srv/SetCurrentVelocity.srv"
  "srv/SetStratifiedCurrentVelocity.srv"
  "srv/SetStratifiedCurrentDirection.srv"
  DEPENDENCIES geometry_msgs
)

# Install message package.xml
install(FILES
  package.xml
  DESTINATION share/${PROJECT_NAME}
)

# Define a variable 'GZ_MSGS_VER' holding the version number
set(GZ_MSGS_VER ${gz-msgs10_VERSION_MAJOR})

# Define a variable 'MSGS_PROTOS' listing the .proto files
set(MSGS_PROTOS ${CMAKE_CURRENT_SOURCE_DIR}/proto/dave_gz_world_plugins_msgs/msgs/StratifiedCurrentVelocity.proto)

# Define the GZ_DATA_INSTALL_DIR variable
# DESTINATION ${GZ_DATA_INSTALL_DIR}/protos
install(
  DIRECTORY gz
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/proto/dave_gz_world_plugins_msgs/msgs/StratifiedCurrentVelocity.proto
  COMPONENT proto
  FILES_MATCHING PATTERN "*.proto")

# Example of custom messages that depend on gz.msgs
set(MSGS_PROTOS
  ${CMAKE_CURRENT_SOURCE_DIR}/proto/dave_gz_world_plugins_msgs/msgs/StratifiedCurrentVelocity.proto)
gz_msgs_generate_messages(
  # The cmake target to be generated for libraries/executables to link
  TARGET msgs    # pressure_sensor_msgs_gen
  # The protobuf package to generate (Typically based on the path)
  PROTO_PACKAGE "dave_gz_world_plugins_msgs.msgs"
  # The path to the base directory of the proto files
  # All import paths should be relative to this (eg gz/custom_msgs/vector3d.proto)
  MSGS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/proto
  # List of proto files to generate
  MSGS_PROTOS ${MSGS_PROTOS}
  # List of message targets this library imports from
  # DEPENDENCIES gz_msgs_gen
  # Dependency on gz-msgs
  DEPENDENCIES gz-msgs${GZ_MSGS_VER}::gz-msgs${GZ_MSGS_VER}
)

ament_export_dependencies(rosidl_default_runtime)
# Install CMake package configuration
ament_export_include_directories(include)

ament_package()
