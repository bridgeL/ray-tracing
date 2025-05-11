cmake_minimum_required(VERSION 3.10)
project(Rasterizer)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) # 默认使用Release模式
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
add_executable(main main.cc)
target_link_libraries(main ${OpenCV_LIBRARIES})