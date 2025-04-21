cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

# 默认Debug模式，通过命令行参数 -DCMAKE_BUILD_TYPE=Release 切换
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build mode: ${CMAKE_BUILD_TYPE}")

# 根据模式设置不同编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_options(-O3 -march=native -DNDEBUG)
    add_compile_definitions(EIGEN_NO_DEBUG)
endif()

set(CMAKE_CXX_STANDARD 17)

find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)

include_directories(${EIGEN3_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
add_executable(task1 main.cc)
target_link_libraries(task1 ${OpenCV_LIBRARIES})