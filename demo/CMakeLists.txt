cmake_minimum_required(VERSION 3.15)
project(demo)

set(CMAKE_CXX_STANDARD 14)

find_package(Jibal 0.3.0 REQUIRED)

add_executable(demo demo.cpp)
target_link_libraries(demo jibal)
