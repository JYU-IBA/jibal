cmake_minimum_required(VERSION 3.15)
include(GenerateExportHeader)
project(jibal C)

set(jibal_VERSION 0.0.1)

set(CMAKE_C_STANDARD 99)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
add_library(jibal SHARED get_stop.c gsto.c masses.c phys.c units.c material.c layer.c kin.c jibal_masses.h jibal_material.h jibal_gsto.h jibal_phys.h jibal_units.h jibal_layer.h jibal_kin.h)

generate_export_header(jibal)
set_property(TARGET jibal PROPERTY VERSION ${jibal_VERSION})
set_property(TARGET jibal PROPERTY SOVERSION 0)
set_property(TARGET jibal PROPERTY
        INTERFACE_jibal_MAJOR_VERSION 0)
set_property(TARGET jibal APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING jibal_MAJOR_VERSION)

install(TARGETS jibal EXPORT jibaltargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )
install(
        FILES
        jibal_gsto.h
        "${CMAKE_CURRENT_BINARY_DIR}/jibal_export.h"
        DESTINATION
        include
        COMPONENT
        Devel
)
