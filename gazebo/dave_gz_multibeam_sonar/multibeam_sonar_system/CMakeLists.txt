cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)
project(multibeam_sonar_system)

find_package(ament_cmake REQUIRED)
find_package(gz-cmake3 REQUIRED)
find_package(gz-common5 REQUIRED COMPONENTS profiler)
find_package(gz-plugin2 REQUIRED COMPONENTS register)
find_package(gz-rendering8 REQUIRED OPTIONAL_COMPONENTS ogre ogre2)
find_package(gz-sim8 REQUIRED)
find_package(gz-sensors8 REQUIRED)

if(TARGET gz-rendering8::ogre)
  add_definitions(-DWITH_OGRE)
endif()

if(TARGET gz-rendering8::ogre2)
  add_definitions(-DWITH_OGRE2)
endif()

add_subdirectory(../multibeam_sonar ${CMAKE_BINARY_DIR}/multibeam_sonar)

add_library(${PROJECT_NAME} SHARED MultibeamSonarSystem.cc)
target_link_libraries(${PROJECT_NAME}
gz-common5::gz-common5
gz-plugin2::gz-plugin2
gz-sim8::gz-sim8
gz-sensors8::gz-sensors8
  multibeam_sonar
)
target_include_directories(${PROJECT_NAME}
  PUBLIC
    ../multibeam_sonar
)

# Install targets
install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

# Environment hooks
ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

ament_package()
