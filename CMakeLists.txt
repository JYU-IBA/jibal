cmake_minimum_required(VERSION 3.15)
if(WIN32 AND DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
  message(STATUS "CMake toolchain file ${CMAKE_TOOLCHAIN_FILE}")
endif()
project(jibal
        VERSION 0.2.3
        DESCRIPTION "JIBAL - Jyväskylä ion beam analysis library"
        LANGUAGES C
        )
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()
add_subdirectory(jibal)
add_subdirectory(tools)
#add_subdirectory(demo)
