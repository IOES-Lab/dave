cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)
project(multibeam_sonar_system)

find_package(ament_cmake REQUIRED)
find_package(CUDAToolkit QUIET)

if(CUDAToolkit_FOUND)
  find_package(tinyxml2 CONFIG REQUIRED)
  find_package(gz-cmake 5 REQUIRED)
  find_package(gz-common 7 REQUIRED COMPONENTS profiler)
  find_package(gz-plugin 4 REQUIRED COMPONENTS register)
  find_package(gz-rendering 10 REQUIRED OPTIONAL_COMPONENTS ogre ogre2)
  find_package(gz-sim 10 REQUIRED)
  find_package(gz-sensors 10 REQUIRED)

  if(TARGET gz-rendering::ogre)
    set(HAVE_OGRE TRUE)
    add_definitions(-DWITH_OGRE)
  endif()

  if(TARGET gz-rendering::ogre2)
    set(HAVE_OGRE2 TRUE)
    add_definitions(-DWITH_OGRE2)
  endif()

  add_subdirectory(../multibeam_sonar ${CMAKE_BINARY_DIR}/multibeam_sonar)

  add_library(${PROJECT_NAME} SHARED MultibeamSonarSystem.cc)
  target_link_libraries(${PROJECT_NAME}
      gz-common::gz-common
      gz-plugin::gz-plugin
      gz-sim::gz-sim
      gz-sensors::gz-sensors
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

else()
  message(STATUS "CUDA Toolkit not found: Skipping CUDA-specific targets")
endif()

ament_package()
