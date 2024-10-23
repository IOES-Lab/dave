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
find_package(gz-cmake3 REQUIRED)
find_package(gz-msgs10 REQUIRED)

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

ament_export_dependencies(rosidl_default_runtime)
# Install CMake package configuration
ament_export_include_directories(include)

ament_package()