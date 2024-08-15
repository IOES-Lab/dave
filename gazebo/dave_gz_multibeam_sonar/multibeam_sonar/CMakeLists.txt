cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(multibeam_sonar)

find_package(ament_cmake REQUIRED)
find_package(gz-cmake3 REQUIRED)
find_package(gz-sensors8 REQUIRED)

add_library(${PROJECT_NAME} SHARED MultibeamSonar.cc)
target_link_libraries(${PROJECT_NAME}
  PUBLIC gz-sensors8::gz-sensors8)

# Install targets
install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

ament_target_dependencies(${PROJECT_NAME})

# Environment hooks
ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

ament_package()