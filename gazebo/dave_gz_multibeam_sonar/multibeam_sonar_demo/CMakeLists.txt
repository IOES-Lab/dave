cmake_minimum_required(VERSION 3.8)
project(dave_multibeam_sonar_demo)

find_package(ament_cmake REQUIRED)
find_package(CUDAToolkit QUIET)

if(CUDAToolkit_FOUND)
  install(
    DIRECTORY launch
    DESTINATION share/${PROJECT_NAME}
  )

  install(
    DIRECTORY rviz
    DESTINATION share/${PROJECT_NAME}
  )

  ament_environment_hooks(
    "${CMAKE_CURRENT_SOURCE_DIR}/hooks/${PROJECT_NAME}.dsv.in")

else()
  message(STATUS "CUDA Toolkit not found: Skipping CUDA-specific targets")
endif()

ament_package()
