cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)

# Suppress developer warnings for the entire workspace.

set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS TRUE CACHE BOOL
    "Suppress developer warnings" FORCE)
# Set CMP0144 to NEW to ensure find_package
# uses upper-case <PACKAGENAME>_ROOT variables.

cmake_policy(SET CMP0144 NEW)

set(CUDA_ARCHITECTURE "native" CACHE STRING "Target CUDA SM version(s), e.g. native or 70;80;89")
set(CMAKE_CUDA_ARCHITECTURES "${CUDA_ARCHITECTURE}")

project(multibeam_sonar)

find_package(ament_cmake REQUIRED)
find_package(gz-cmake3 REQUIRED)
find_package(gz-sim8 REQUIRED)
find_package(gz-sensors8 REQUIRED)
find_package(gz-rendering8 REQUIRED OPTIONAL_COMPONENTS ogre ogre2)
find_package(gz-transport13 REQUIRED)
find_package(gz-msgs10 REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(marine_acoustic_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(wgpu_vendor REQUIRED)
find_package(CUDAToolkit QUIET)

if(CUDAToolkit_FOUND)
  set(CMAKE_CUDA_COMPILER "${CUDAToolkit_NVCC_EXECUTABLE}" CACHE FILEPATH "CUDA compiler" FORCE)
  enable_language(CUDA)
  find_package(CUDA REQUIRED)
  message(STATUS "CUDA ${CUDAToolkit_VERSION} found (SM ${CUDA_ARCHITECTURE}), enabling CUDA support.")
  include_directories(${CUDA_INCLUDE_DIRS})
endif()

if(TARGET gz-rendering::ogre)
  add_definitions(-DWITH_OGRE)
endif()
if(TARGET gz-rendering::ogre2)
  add_definitions(-DWITH_OGRE2)
endif()

set(SONAR_SOURCES
  MultibeamSonarSensor.cc
  sonar_compute_cpu.cc
  sonar_compute_wgpu.cc
  sonar_compute_cuda.cc
)

if(CUDAToolkit_FOUND)
  list(APPEND SONAR_SOURCES sonar_calculation_cuda.cu)
endif()

add_library(${PROJECT_NAME} SHARED ${SONAR_SOURCES})

if(CUDAToolkit_FOUND)
  set_target_properties(${PROJECT_NAME}
    PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
  )
  target_compile_definitions(${PROJECT_NAME} PRIVATE DAVE_HAS_CUDA_BACKEND=1)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
  ${gz-sensors8_INCLUDE_DIRS}
  ${gz-rendering8_INCLUDE_DIRS}
  ${gz-msgs10_INCLUDE_DIRS}
  ${gz-transport13_INCLUDE_DIRS}
  ${gz-sim8_INCLUDE_DIRS}
  ${rclcpp_INCLUDE_DIRS}
  ${sensor_msgs_INCLUDE_DIRS}
  ${geometry_msgs_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIRS}
  ${marine_acoustic_msgs_INCLUDE_DIRS}
  ${cv_bridge_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  gz-sim8::gz-sim8
  gz-sensors8::gz-sensors8
  gz-msgs10::gz-msgs10
  gz-transport13::gz-transport13
  gz-rendering8::gz-rendering8
  ${rclcpp_LIBRARIES}
  ${sensor_msgs_LIBRARIES}
  ${geometry_msgs_LIBRARIES}
  ${OpenCV_LIBS}
  Eigen3::Eigen
  ${marine_acoustic_msgs_LIBRARIES}
)

if(APPLE)
  target_link_libraries(${PROJECT_NAME}
    "-framework Metal"
    "-framework QuartzCore"
  )
endif()

if(TARGET cv_bridge::cv_bridge)
  target_link_libraries(${PROJECT_NAME} cv_bridge::cv_bridge)
elseif(TARGET cv_bridge)
  target_link_libraries(${PROJECT_NAME} cv_bridge)
elseif(cv_bridge_LIBRARIES)
  target_link_libraries(${PROJECT_NAME} ${cv_bridge_LIBRARIES})
endif()

if(CUDAToolkit_FOUND)
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
    ${CUDA_LIBRARIES}
    ${CUDA_CUFFT_LIBRARIES}
    ${CUBLAS_LIB}
  )
endif()

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

if(TARGET gz-sensors::gz-sensors8-rendering)
  target_link_libraries(${PROJECT_NAME} gz-sensors::gz-sensors8-rendering)
else()
  find_library(GZ_SENSORS_RENDERING_LIB gz-sensors8-rendering)
  if(GZ_SENSORS_RENDERING_LIB)
    target_link_libraries(${PROJECT_NAME} ${GZ_SENSORS_RENDERING_LIB})
  endif()
endif()

install(TARGETS ${PROJECT_NAME} DESTINATION lib/${PROJECT_NAME})

ament_environment_hooks(
  "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

ament_package()
