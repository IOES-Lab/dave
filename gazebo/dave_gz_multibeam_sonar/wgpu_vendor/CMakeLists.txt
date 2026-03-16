cmake_minimum_required(VERSION 3.11)
project(wgpu_vendor)

find_package(ament_cmake REQUIRED)
find_package(Vulkan REQUIRED)

find_program(CARGO_EXECUTABLE cargo REQUIRED)

set(RUST_CRATE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/sonar_wgpu_rust)
set(RUST_TARGET_DIR ${CMAKE_CURRENT_BINARY_DIR}/cargo_target)
set(RUST_PROFILE_DIR ${RUST_TARGET_DIR}/release)
set(RUST_LIB ${RUST_PROFILE_DIR}/libsonar_wgpu.a)

# Track all Rust source files so colcon/CMake triggers cargo when they change.
file(GLOB_RECURSE RUST_SOURCES
  ${RUST_CRATE_DIR}/src/*.rs
  ${RUST_CRATE_DIR}/src/shaders/*.wgsl
  ${RUST_CRATE_DIR}/Cargo.toml
  ${RUST_CRATE_DIR}/Cargo.lock
)

add_custom_command(
  OUTPUT ${RUST_LIB}
  COMMAND ${CARGO_EXECUTABLE} build --release --target-dir ${RUST_TARGET_DIR}
  WORKING_DIRECTORY ${RUST_CRATE_DIR}
  DEPENDS ${RUST_SOURCES}
  COMMENT "Building sonar_wgpu Rust static library"
  VERBATIM
)

add_custom_target(sonar_wgpu_rust ALL DEPENDS ${RUST_LIB})

add_library(sonar_wgpu_rust_static STATIC IMPORTED GLOBAL)
set_target_properties(sonar_wgpu_rust_static PROPERTIES
  IMPORTED_LOCATION ${RUST_LIB})
add_dependencies(sonar_wgpu_rust_static sonar_wgpu_rust)

add_library(wgpu_vendor INTERFACE)
add_dependencies(wgpu_vendor sonar_wgpu_rust_static)

target_link_libraries(wgpu_vendor INTERFACE sonar_wgpu_rust_static dl pthread m Vulkan::Vulkan)

add_library(wgpu::wgpu_vendor ALIAS wgpu_vendor)

install(FILES ${RUST_LIB} DESTINATION lib)

install(TARGETS wgpu_vendor EXPORT wgpu_vendorTargets)
install(EXPORT wgpu_vendorTargets
  NAMESPACE wgpu::
  FILE wgpu_vendorTargets.cmake
  DESTINATION share/${PROJECT_NAME}/cmake)

ament_export_targets(wgpu_vendorTargets HAS_LIBRARY_TARGET)
ament_export_dependencies(Vulkan)
ament_package()
