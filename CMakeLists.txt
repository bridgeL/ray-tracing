cmake_minimum_required(VERSION 3.14)
project(RayTracing)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenMP REQUIRED)

macro(smart_find_package name install)
    find_package(${name} QUIET)
    if(NOT ${name}_FOUND)
        message(STATUS "E: 未找到系统安装的 ${name}，请通过 ${install} 安装")
    else()
        message(STATUS "找到系统安装的 ${name}")
    endif()
endmacro()

# 智能获取
smart_find_package(OpenCV "sudo apt-get install libopencv-dev")
smart_find_package(Eigen3 "sudo apt-get install libeigen3-dev")

# 编译优化
add_compile_options(
    -O3                  # 最高优化级别
    -march=native        # 针对当前 CPU 架构优化
    -DNDEBUG             # 禁用断言
)
add_compile_definitions(EIGEN_NO_DEBUG)

# 创建可执行文件
add_executable(main main.cc)

# Eigen3 包含目录处理
target_include_directories(main PRIVATE ${EIGEN3_INCLUDE_DIRS})

# OpenCV 链接处理
target_link_libraries(main PRIVATE ${OpenCV_LIBS})

# 添加OpenMP支持
target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
