cmake_minimum_required(VERSION 3.8)
project(dave_gz_sensor_plugins)

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
find_package(sensor_msgs REQUIRED)
find_package(Protobuf REQUIRED)
find_package(gz-msgs10 REQUIRED)

# Set version variables
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})
set(GZ_MSGS_VER ${gz-msgs10_VERSION_MAJOR})

message(STATUS "Compiling against Gazebo Harmonic")

add_library(UsblTransceiver SHARED src/UsblTransceiver.cc)
add_library(UsblTransponder SHARED src/UsblTransponder.cc)
add_library(sea_pressure_sensor SHARED src/sea_pressure_sensor.cc)

target_include_directories(UsblTransceiver PRIVATE include)
target_include_directories(UsblTransponder PRIVATE include)
target_include_directories(sea_pressure_sensor PRIVATE include)

target_link_libraries(UsblTransceiver
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

target_link_libraries(UsblTransponder
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

target_link_libraries(sea_pressure_sensor
  gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
)

# Specify dependencies for FullSystem using ament_target_dependencies
ament_target_dependencies(UsblTransceiver
  dave_interfaces
  sensor_msgs
  rclcpp
  geometry_msgs
  std_msgs
)

ament_target_dependencies(UsblTransponder
  dave_interfaces
  rclcpp
  std_msgs
)

ament_target_dependencies(sea_pressure_sensor
  rclcpp
  std_msgs
  sensor_msgs
  geometry_msgs
)

# Install targets
install(TARGETS UsblTransceiver UsblTransponder sea_pressure_sensor
  DESTINATION lib/${PROJECT_NAME}
)

# Install headers
install(DIRECTORY include/
  DESTINATION include/
)

# Environment hooks
ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

# find_package(CUDAToolkit QUIET)
# if (CUDAToolkit_FOUND)
#   enable_language(CUDA)
#   message(STATUS "CUDA found, enabling CUDA support.")
#   include_directories(${CUDA_INCLUDE_DIRS})
#   set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=sm_60")

#   find_package(cv_bridge REQUIRED)
#   find_package(OpenCV REQUIRED)
#   find_package(rosidl_default_generators REQUIRED)
#   find_package(PCL REQUIRED)
#   find_package(pcl_ros REQUIRED)
#   find_package(marine_acoustic_msgs REQUIRED)
#   find_package(gz-sensors${GZ_SIM_VER} REQUIRED)
#   set(GZ_SENSORS_VER ${gz-sensors8_VERSION_MAJOR})

#   add_library(MultibeamSonarSystem SHARED
#               src/MultibeamSonarSystem.cc
#   )

#   set_target_properties(MultibeamSonarSystem
#                         PROPERTIES CUDA_SEPARABLE_COMPILATION ON)

#   target_include_directories(MultibeamSonarSystem PRIVATE include)
#   target_link_libraries(MultibeamSonarSystem
#                         gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER}
#                         gz-sensors${GZ_SENSORS_VER}::gz-sensors${GZ_SENSORS_VER}
#                         gz-plugin${GZ_PLUGIN_VER}::gz-plugin${GZ_PLUGIN_VER}
#                         ${CUDA_LIBRARIES}
#                         ${PCL_LIBRARIES}
#                         ${OpenCV_LIBRARIES}
#   )

#   ament_target_dependencies(MultibeamSonarSystem
#                             rclcpp
#                             std_msgs
#                             sensor_msgs
#                             geometry_msgs
#                             PCL
#                             OpenCV
#                             marine_acoustic_msgs
#   )
#   install(TARGETS MultibeamSonarSystem
#           DESTINATION lib/${PROJECT_NAME}
#   )
# else()
#   message(STATUS "CUDA Toolkit not found: Skipping CUDA-specific targets")
# endif()

# Testing setup
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
