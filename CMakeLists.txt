cmake_minimum_required(VERSION 3.15)
include(version.cmake)
if(WIN32 AND DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
  message(STATUS "CMake toolchain file ${CMAKE_TOOLCHAIN_FILE}")
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
project(Jibal
        VERSION "${BUILD_VERSION}"
        DESCRIPTION "JIBAL - Jyväskylä ion beam analysis library"
        LANGUAGES C
        )
string(TIMESTAMP timestamp "%Y-%m-%d" UTC)
set(default_build_type "Release")
#if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
#  set(default_build_type "Debug")
#endif()

option(DEBUG_MODE_ENABLE "Enable debug mode" OFF)
if(DEBUG_MODE_ENABLE)
  if (MSVC)
    add_compile_options(/W4 /DDEBUG)
  else()
    add_compile_options(-Wall -Wextra -pedantic -DDEBUG)
  endif()
endif()

add_subdirectory(jibal)
add_subdirectory(tools)
