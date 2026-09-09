cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)

# Suppress developer warnings for the entire workspace.

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE BOOL
    "Suppress developer warnings" FORCE)
# Set CMP0144 to NEW to ensure find_package
# uses upper-case <PACKAGENAME>_ROOT variables.

cmake_policy(SET CMP0144 NEW)

project(multibeam_sonar)

set(CUDA_ARCHITECTURE "60" CACHE STRING "Target CUDA SM version")

find_package(ament_cmake REQUIRED)
find_package(CUDAToolkit QUIET)

if(CUDAToolkit_FOUND)

  enable_language(CUDA)
  find_package(CUDA REQUIRED)
  message(STATUS "CUDA found, enabling CUDA support.")
  include_directories(${CUDA_INCLUDE_DIRS})
  set(CUDA_NVCC_FLAGS "${CUDA_NVCC_FLAGS} -arch=sm_${CUDA_ARCHITECTURE}")
  find_package(tinyxml2 CONFIG REQUIRED)
  find_package(gz-cmake 5 REQUIRED)
  find_package(gz-sim 10 REQUIRED)
  find_package(gz-sensors 10 REQUIRED)
  find_package(gz-rendering 10 REQUIRED OPTIONAL_COMPONENTS ogre ogre2)
  find_package(gz-transport 15 REQUIRED)
  find_package(gz-msgs 12 REQUIRED)
  find_package(rclcpp REQUIRED)
  find_package(sensor_msgs REQUIRED)
  find_package(geometry_msgs REQUIRED)
  find_package(rosidl_default_generators REQUIRED)
  find_package(OpenCV REQUIRED)
  find_package(marine_acoustic_msgs REQUIRED)
  find_package(cv_bridge REQUIRED)

  if(TARGET gz-rendering::ogre)
    set(HAVE_OGRE TRUE)
    add_definitions(-DWITH_OGRE)
  endif()

  if(TARGET gz-rendering::ogre2)
    set(HAVE_OGRE2 TRUE)
    add_definitions(-DWITH_OGRE2)
  endif()

  add_library(${PROJECT_NAME}
    SHARED
      MultibeamSonarSensor.cc
      sonar_calculation_cuda.cu
  )

  set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
  )

  target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${gz-sensors_INCLUDE_DIRS}
    ${gz-rendering_INCLUDE_DIRS}
    ${gz-msgs_INCLUDE_DIRS}
    ${CUDA_INCLUDE_DIRS}
    ${gz-transport_INCLUDE_DIRS}
    ${gz-sim_INCLUDE_DIRS}
  )

  find_library(CUBLAS_LIB cublas
    HINTS
      ${CUDAToolkit_LIBRARY_DIR}
      /usr/local/cuda/lib64
      /usr/lib/x86_64-linux-gnu
      /usr/lib
  )
  if(NOT CUBLAS_LIB)
    message(FATAL_ERROR "cuBLAS not found")
  endif()

  target_link_libraries(${PROJECT_NAME}
    gz-sim::gz-sim
    gz-sensors::gz-sensors
    gz-msgs::gz-msgs
    gz-transport::gz-transport
    gz-rendering::gz-rendering
    ${CUDA_LIBRARIES}
    ${CUDA_CUFFT_LIBRARIES}
    ${CUBLAS_LIB}
  )

  if(TARGET gz-sensors::gz-sensors-rendering)
    target_link_libraries(${PROJECT_NAME} gz-sensors::gz-sensors-rendering)
  endif()

  install(TARGETS ${PROJECT_NAME}
    DESTINATION lib/${PROJECT_NAME}
  )

  ament_target_dependencies(${PROJECT_NAME}
    rclcpp
    sensor_msgs
    rosidl_default_generators
    OpenCV
    marine_acoustic_msgs
    geometry_msgs
    cv_bridge
  )

  ament_environment_hooks(
    "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in"
  )

else()
  message(STATUS "CUDA Toolkit not found or disabled: "
                "Skipping CUDA-specific targets")
endif()

ament_package()
