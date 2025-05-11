cmake_minimum_required(VERSION 3.10)
project(RayTracing)

# 设置默认构建类型（如果未指定）
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)  # 默认使用Release模式
endif()

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找OpenCV
find_package(OpenCV REQUIRED)
message(STATUS "OpenCV version: ${OpenCV_VERSION}")

# 添加可执行文件
add_executable(main main.cc)

# 链接OpenCV库
target_link_libraries(main ${OpenCV_LIBS})

# 根据构建类型设置编译选项
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Release模式：启用O3优化和向量化
    target_compile_options(main PRIVATE
        -O3
        -march=native  # 启用本地CPU架构优化（自动向量化）
        -ffast-math    # 快速数学优化（可能牺牲精度）
    )
    message(STATUS "Build type: Release (O3 optimization enabled)")
elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug模式：禁用优化，添加调试信息
    target_compile_options(main PRIVATE
        -O0
        -g            # 生成调试符号
        -Wall         # 启用所有警告
    )
    message(STATUS "Build type: Debug (no optimization)")
endif()