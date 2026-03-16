cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)

# Suppress developer warnings for the entire workspace.

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE BOOL
    "Suppress developer warnings" FORCE)
# Set CMP0144 to NEW to ensure find_package
# uses upper-case <PACKAGENAME>_ROOT variables.

cmake_policy(SET CMP0144 NEW)

project(multibeam_sonar)

find_package(ament_cmake REQUIRED)
find_package(gz-cmake REQUIRED)
find_package(gz-sim REQUIRED)
find_package(gz-sensors REQUIRED)
find_package(gz-rendering REQUIRED OPTIONAL_COMPONENTS ogre ogre2)
find_package(gz-transport REQUIRED)
find_package(gz-msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(marine_acoustic_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(wgpu_vendor REQUIRED)

if(TARGET gz-rendering::ogre)
  add_definitions(-DWITH_OGRE)
endif()
if(TARGET gz-rendering::ogre2)
  add_definitions(-DWITH_OGRE2)
endif()

add_library(${PROJECT_NAME} SHARED
  MultibeamSonarSensor.cc
  sonar_compute_cpu.cc
  sonar_compute_wgpu.cc
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${gz-sensors_INCLUDE_DIRS}
  ${gz-rendering_INCLUDE_DIRS}
  ${gz-msgs_INCLUDE_DIRS}
  ${gz-transport_INCLUDE_DIRS}
  ${gz-sim_INCLUDE_DIRS}
  ${rclcpp_INCLUDE_DIRS}
  ${sensor_msgs_INCLUDE_DIRS}
  ${geometry_msgs_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${marine_acoustic_msgs_INCLUDE_DIRS}
  ${cv_bridge_INCLUDE_DIRS}
)

if(DEFINED ENV{ROS_DISTRO} AND EXISTS "/opt/ros/$ENV{ROS_DISTRO}/include/cv_bridge")
  target_include_directories(${PROJECT_NAME} PUBLIC "/opt/ros/$ENV{ROS_DISTRO}/include/cv_bridge")
elseif(EXISTS "/opt/ros/rolling/include/cv_bridge")
  target_include_directories(${PROJECT_NAME} PUBLIC "/opt/ros/rolling/include/cv_bridge")
endif()

target_link_libraries(${PROJECT_NAME}
  gz-sim::gz-sim
  gz-sensors::gz-sensors
  gz-msgs::gz-msgs
  gz-transport::gz-transport
  gz-rendering::gz-rendering
  ${rclcpp_LIBRARIES}
  ${sensor_msgs_LIBRARIES}
  ${geometry_msgs_LIBRARIES}
  ${OpenCV_LIBS}
  ${marine_acoustic_msgs_LIBRARIES}
)

if(TARGET wgpu::wgpu_vendor)
  target_link_libraries(${PROJECT_NAME} wgpu::wgpu_vendor)
elseif(TARGET wgpu_vendor)
  target_link_libraries(${PROJECT_NAME} wgpu_vendor)
endif()

# Ensure the Rust static archive is linked into this shared library.
# The exported interface target may not always propagate non-installed imported
# targets across package boundaries in overlay builds.
if(DEFINED wgpu_vendor_DIR)
  get_filename_component(_wgpu_vendor_prefix "${wgpu_vendor_DIR}/../../.." ABSOLUTE)
  set(_sonar_wgpu_static "${_wgpu_vendor_prefix}/lib/libsonar_wgpu.a")
  if(EXISTS "${_sonar_wgpu_static}")
    target_link_libraries(${PROJECT_NAME} "${_sonar_wgpu_static}")
  endif()
endif()

if(TARGET cv_bridge::cv_bridge)
  target_link_libraries(${PROJECT_NAME} cv_bridge::cv_bridge)
elseif(TARGET cv_bridge)
  target_link_libraries(${PROJECT_NAME} cv_bridge)
else()
  target_link_libraries(${PROJECT_NAME} ${cv_bridge_LIBRARIES})
endif()

if(TARGET gz-sensors::gz-sensors-rendering)
  target_link_libraries(${PROJECT_NAME} gz-sensors::gz-sensors-rendering)
else()
  find_library(GZ_SENSORS_RENDERING_LIB gz-sensors-rendering)
  if(GZ_SENSORS_RENDERING_LIB)
    target_link_libraries(${PROJECT_NAME} ${GZ_SENSORS_RENDERING_LIB})
  endif()
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

ament_package()
