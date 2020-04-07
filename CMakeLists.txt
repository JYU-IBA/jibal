cmake_minimum_required(VERSION 3.15)
project(jibal
        VERSION 0.1.2
        DESCRIPTION "JIBAL - Jyväskylä ion beam analysis library"
        LANGUAGES C
        )
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()
add_subdirectory(jibal)
add_subdirectory(aux)
#add_subdirectory(demo)
