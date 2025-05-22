cmake_minimum_required(VERSION 3.14)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)

macro(smart_find_package name install)
    find_package(${name} QUIET)
    if(NOT ${name}_FOUND)
        message(STATUS "Error: System installation of ${name} not found, please install via: ${install}")
    else()
        message(STATUS "Found system installation of ${name}")
    endif()
endmacro()

# Smart package finder with installation instructions
smart_find_package(OpenCV "sudo apt-get install libopencv-dev")
smart_find_package(Eigen3 "sudo apt-get install libeigen3-dev")

# Compilation optimization flags
add_compile_options(
    -O3                  # Maximum optimization level
    -march=native        # Optimize for current CPU architecture
    -DNDEBUG             # Disable assertions
)
add_compile_definitions(EIGEN_NO_DEBUG)

# Create executable
add_executable(main main.cc)

# Eigen3 include directories
target_include_directories(main PRIVATE ${EIGEN3_INCLUDE_DIRS})

# OpenCV linking
target_link_libraries(main PRIVATE ${OpenCV_LIBS})

# Add OpenMP support
target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
