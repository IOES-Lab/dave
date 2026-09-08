cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)

if(POLICY CMP0100)
  cmake_policy(SET CMP0100 NEW)
endif()

project(WavesControl)

set(CMAKE_AUTOMOC ON)

find_package(gz-sim8 REQUIRED COMPONENTS gui)
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})

qt5_add_resources(resources_RCC ${PROJECT_NAME}.qrc)

add_library(${PROJECT_NAME} SHARED
  ${PROJECT_NAME}.cc
  ${resources_RCC}
)
target_link_libraries(${PROJECT_NAME}
  PRIVATE gz-sim${GZ_SIM_VER}::gui
)
