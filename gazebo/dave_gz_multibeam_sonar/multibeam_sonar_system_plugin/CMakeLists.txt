cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)
project(multibeam_sonar_system_plugin)

find_package(ament_cmake REQUIRED)
find_package(gz-cmake3 REQUIRED)

gz_find_package(gz-plugin2 REQUIRED COMPONENTS register)
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})

gz_find_package(gz-sim8 REQUIRED)
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})

find_package(gz-sensors8 REQUIRED)
set(GZ_SENSORS_VER ${gz-sensors8_VERSION_MAJOR})

add_subdirectory(../multibeam_sonar ${CMAKE_BINARY_DIR}/multibeam_sonar)

add_library(${PROJECT_NAME} SHARED MultibeamSonarSystemPlugin.cc)
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    gz-plugin${GZ_PLUGIN_VER}::gz-plugin${GZ_PLUGIN_VER}
    gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
    gz-sensors${GZ_SENSORS_VER}::gz-sensors${GZ_SENSORS_VER}
    multibeam_sonar
)
target_include_directories(${PROJECT_NAME}
    PUBLIC ../multibeam_sonar)

target_include_directories(${PROJECT_NAME} PRIVATE include)

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