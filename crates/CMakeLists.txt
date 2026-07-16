cmake_minimum_required(VERSION 3.25)

# Build Rust library using cargo
set(CARGO_RELEASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/target/release")
set(CARGO_DEBUG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/target/debug")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CARGO_BUILD_DIR "${CARGO_RELEASE_DIR}")
    set(CARGO_BUILD_FLAG "--release")
else()
    set(CARGO_BUILD_DIR "${CARGO_DEBUG_DIR}")
    set(CARGO_BUILD_FLAG "")
endif()

# Determine library names
set(RUST_STATIC_LIB "${CARGO_BUILD_DIR}/libcore.a")
set(RUST_SHARED_LIB "${CARGO_BUILD_DIR}/libcore.so")

# Custom command to build Rust library
add_custom_command(
    OUTPUT "${RUST_STATIC_LIB}" "${RUST_SHARED_LIB}"
    COMMAND cargo build ${CARGO_BUILD_FLAG}
    WORKING_DIRECTORY "${CARGO_BUILD_DIR}/.."
    COMMENT "Building Rust launcher-core library"
    VERBATIM
)

# Import the Rust static library
add_library(launcher_core_static STATIC IMPORTED)
set_target_properties(launcher_core_static PROPERTIES
    IMPORTED_LOCATION "${RUST_STATIC_LIB}"
)

# Create an interface library for the Rust core
add_library(launcher_core INTERFACE)
target_include_directories(launcher_core INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(launcher_core INTERFACE launcher_core_static)

# Link system libraries needed by Rust
find_package(Threads REQUIRED)
target_link_libraries(launcher_core INTERFACE Threads::Threads dl m)

# Add custom target for cargo build
add_custom_target(cargo_build DEPENDS "${RUST_STATIC_LIB}" "${RUST_SHARED_LIB}")
add_dependencies(launcher_core cargo_build)
