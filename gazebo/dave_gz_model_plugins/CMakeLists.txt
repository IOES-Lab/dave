cmake_minimum_required(VERSION 3.8)
project(dave_gz_model_plugins)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(gz-sim8 REQUIRED)
find_package(sdformat14 REQUIRED)
find_package(rclpy REQUIRED)

# Include directories
include_directories(include)

# Set version variables
set(GZ_PLUGIN_VER ${gz-plugin2_VERSION_MAJOR})
set(GZ_COMMON_VER ${gz-common5_VERSION_MAJOR})
set(GZ_SIM_VER ${gz-sim8_VERSION_MAJOR})

# Install Python package
ament_python_install_package(${PROJECT_NAME} PACKAGE_DIR "src")


# add_library(LinearBatteryPlugin SHARED src/linear_battery_plugin.py)

# target_link_libraries(LinearBatteryPlugin
#   gz-sim${GZ_SIM_VER}::gz-sim${GZ_SIM_VER})


# Install Python script directly
install(PROGRAMS
  ${CMAKE_CURRENT_SOURCE_DIR}/test/battery_state_listener.py
  DESTINATION lib/${PROJECT_NAME}
)

install(FILES 
  ${CMAKE_CURRENT_SOURCE_DIR}/test/battery_world.sdf
  DESTINATION share/${PROJECT_NAME}/test/
)

# Install other files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
