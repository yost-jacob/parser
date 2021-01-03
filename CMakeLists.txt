cmake_minimum_required(VERSION 3.17)
project(parser)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra)

add_executable(parser main.cpp)