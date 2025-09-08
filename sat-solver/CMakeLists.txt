cmake_minimum_required(VERSION 3.10)
project(SAT-SOLVER-PROJECT CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 11)

# 设置编译器为g++
set(CMAKE_CXX_COMPILER g++)

# 设置包含目录
include_directories(include)

file(GLOB SOURCES "src/*.cpp")

# 设置输出目录为项目根目录下的bin文件夹
set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${OUTPUT_DIR})

# 创建可执行文件
add_executable(sat_solver ${SOURCES})

# 设置可执行文件输出到bin目录
set_target_properties(sat_solver PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# 设置编译选项
target_compile_options(sat_solver PRIVATE -Wall -g)