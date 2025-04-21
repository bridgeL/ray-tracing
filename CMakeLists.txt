cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

# 读取环境变量（默认Debug模式）
if(DEFINED ENV{BUILD_MODE} AND "$ENV{BUILD_MODE}" STREQUAL "Release")
    set(CMAKE_BUILD_TYPE Release)
    message(STATUS "Build mode: Release (optimized)")
    add_compile_options(-O3 -march=native -DNDEBUG)
else()
    set(CMAKE_BUILD_TYPE Debug)
    message(STATUS "Build mode: Debug (no optimizations)")
endif()

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})

add_executable(task1 main.cc)
target_link_libraries(task1 ${OpenCV_LIBRARIES})

# Eigen优化（仅在Release模式生效）
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(task1 PRIVATE EIGEN_NO_DEBUG)
endif()