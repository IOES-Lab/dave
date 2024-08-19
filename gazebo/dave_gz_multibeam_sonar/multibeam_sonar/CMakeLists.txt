cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(multibeam_sonar)

find_package(ament_cmake REQUIRED)
find_package(gz-cmake3 REQUIRED)
find_package(gz-sim8 REQUIRED)
find_package(gz-sensors8 REQUIRED)
find_package(gz-rendering8 REQUIRED OPTIONAL_COMPONENTS ogre ogre2)
find_package(gz-transport13 REQUIRED)

# Set version variables
set(GZ_MSGS_VER ${gz-msgs10_VERSION_MAJOR})
set(GZ_RENDERING_VER ${gz-rendering8_VERSION_MAJOR})
set(GZ_SENSORS_VER ${gz-sensors8_VERSION_MAJOR})
set(GZ_TRANSPORT_VER ${gz-transport13_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})

if (TARGET gz-rendering${GZ_RENDERING_VER}::ogre)
  set(HAVE_OGRE TRUE)
  set(GZ_RENDERING_TARGET gz-rendering${GZ_RENDERING_VER}-ogre)
  add_definitions(-DWITH_OGRE)
endif()

if (TARGET gz-rendering${GZ_RENDERING_VER}::ogre2)
  set(HAVE_OGRE2 TRUE)
  set(GZ_RENDERING_TARGET gz-rendering${GZ_RENDERING_VER}-ogre2)
  add_definitions(-DWITH_OGRE2)
endif()

add_library(${PROJECT_NAME}
  SHARED
    MultibeamSonarSensor.cc
)

# Ensure that the CameraSensor class is available
target_include_directories(${PROJECT_NAME}
  PUBLIC
  ${gz-sensors${GZ_SENSORS_VER}_INCLUDE_DIRS}
  ${gz-rendering${GZ_RENDERING_VER}_INCLUDE_DIRS}
  ${gz-msgs${GZ_MSGS_VER}_INCLUDE_DIRS}
  ${gz-transport${GZ_TRANSPORT_VER}_INCLUDE_DIRS}
  ${gz-sim${GZ_SIM_VER}_INCLUDE_DIRS}
)

# Install targets
install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME}
)

ament_target_dependencies(${PROJECT_NAME}
  gz-sensors${GZ_SENSORS_VER}
  ${GZ_RENDERING_TARGET}
  gz-msgs${GZ_MSGS_VER}
  gz-transport${GZ_TRANSPORT_VER}
  gz-sim${GZ_SIM_VER}
)

# Environment hooks
ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
)

ament_package()


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