cmake_minimum_required(VERSION 3.15)
if(WIN32 AND DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
  message(STATUS "CMake toolchain file ${CMAKE_TOOLCHAIN_FILE}")
endif()
project(Jibal
        VERSION 0.3.4
        DESCRIPTION "JIBAL - Jyväskylä ion beam analysis library"
        LANGUAGES C
        )
string(TIMESTAMP timestamp "%Y-%m-%d" UTC)
configure_file(CITATION.cff.in CITATION.cff @ONLY)
set(default_build_type "Release")
#if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
#  set(default_build_type "Debug")
#endif()

option(DEBUG_MODE_ENABLE "Enable debug mode" OFF)
if(DEBUG_MODE_ENABLE)
  if (MSVC)
    add_compile_options(/W4 /WX /DDEBUG)
  else()
    add_compile_options(-Wall -Wextra -pedantic -DDEBUG)
  endif()
endif()

add_subdirectory(jibal)
add_subdirectory(tools)
