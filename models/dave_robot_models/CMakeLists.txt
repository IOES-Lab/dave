cmake_minimum_required(VERSION 3.8)
project(dave_robot_models)

find_package(ament_cmake REQUIRED)

install(
  DIRECTORY description launch meshes config
  DESTINATION share/dave_robot_models
)

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

ament_package()
