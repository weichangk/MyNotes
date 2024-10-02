# Cmake 最小版本
cmake_minimum_required(VERSION 3.16)

# 使用 C++14
set(CMAKE_CXX_STANDARD 14)

# 强制要求使用 C++ 标准版本
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找项目头文件
set(MY_INC )
file(GLOB_RECURSE MY_INC "inc/*.h" "inc/*.hpp")

# 查找项目源文件
set(MY_SRC )
file(GLOB_RECURSE MY_SRC "src/*.cpp" "src/*.cc")