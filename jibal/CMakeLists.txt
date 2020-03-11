find_package(PkgConfig) #Not used at the moment, but just to be sure

include(FindGSL) #GSL is a special snowflake.
if(NOT GSL_FOUND)
    message(FATAL_ERROR "GSL not found")
endif()
include(CheckSymbolExists)
set(CMAKE_REQUIRED_LIBRARIES "m")

set(CMAKE_C_STANDARD 99)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(DEVELOPER_MODE_ENABLE "Enable developer mode" OFF)
if(DEVELOPER_MODE_ENABLE)
    set(JIBAL_DATADIR "${PROJECT_SOURCE_DIR}/data")
else()
    set(JIBAL_DATADIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/")
endif()
configure_file(jibal_config.h.in jibal_config.h @ONLY)

set(PKGCONF_REQ_PUB "${PKGCONF_REQ_PUB} gsl") #Sets a variable for pkg-config file generation
configure_file(
        ${PROJECT_SOURCE_DIR}/jibal.pc.in
        ${PROJECT_BINARY_DIR}/jibal.pc
        @ONLY
)

FILE(GLOB DATAFILES "${CMAKE_SOURCE_DIR}/data/*.*")
#message(STATUS "${DATAFILES}")
FILE(GLOB PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/jibal_*.h")

add_library(Jibal SHARED
        gsto.c
        masses.c
        phys.c
        units.c
        material.c
        layer.c
        kin.c
        cross_section.c )


target_link_libraries(Jibal GSL::gsl) #GSL is found by FindGSL.

add_executable(get_stop get_stop.c)
add_executable(get_kin get_kin.c)
target_link_libraries(get_stop Jibal)
target_link_libraries(get_kin Jibal)

target_include_directories(Jibal
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
        )

set_property(TARGET Jibal PROPERTY VERSION ${Jibal_VERSION})
set_property(TARGET Jibal PROPERTY SOVERSION 0)
set_property(TARGET Jibal PROPERTY
        INTERFACE_Jibal_MAJOR_VERSION 0)
set_property(TARGET Jibal APPEND PROPERTY
        COMPATIBLE_INTERFACE_STRING Jibal_MAJOR_VERSION)

install(TARGETS Jibal EXPORT JibalTargets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include
        )

install(TARGETS get_stop get_kin RUNTIME DESTINATION bin)

# install headers, note there's a subdirectory
install(
        FILES ${PUBLIC_HEADERS}
        DESTINATION include/${PROJECT_NAME}
)

# install bundled data
install(
        FILES ${DATAFILES}
        DESTINATION share/${PROJECT_NAME}
)

# install exports
install(EXPORT JibalTargets
        FILE JibalTargets.cmake
        DESTINATION lib/cmake/Jibal
        )


# install pkg-config file
install(FILES ${CMAKE_BINARY_DIR}/jibal.pc
        DESTINATION lib/pkgconfig)

include(CMakePackageConfigHelpers)

# config file from a template, includes exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/JibalConfig.cmake"
        INSTALL_DESTINATION "lib/cmake/Jibal"
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
        )

# generate the version file for config
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/JibalConfigVersion.cmake"
        VERSION "${Jibal_VERSION_MAJOR}.${Jibal_VERSION_MINOR}"
        COMPATIBILITY AnyNewerVersion
)

# install the configuration file to binary directory too so "installing" the library is not necessary
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/JibalConfig.cmake
        DESTINATION lib/cmake/Jibal
        )

# install the targets to binary directory
export(EXPORT JibalTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/JibalTargets.cmake"
        )

# cpack configuration
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "${Jibal_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Jibal_VERSION_MINOR}")
include(CPack)
