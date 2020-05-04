#find_package(PkgConfig)

include(FindGSL) #GSL is a special snowflake.
if(NOT GSL_FOUND)
    message(FATAL_ERROR "GSL not found")
endif()
include(CheckSymbolExists)
set(CMAKE_REQUIRED_LIBRARIES "m")

set(CMAKE_C_STANDARD 99)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

option(DEBUG_OUTPUT_ENABLE "Enable debug output" OFF)
if(DEBUG_OUTPUT_ENABLE)
add_compile_options(-DDEBUG)
endif()

option(DEVELOPER_MODE_ENABLE "Enable developer mode" OFF)
if(DEVELOPER_MODE_ENABLE)
    set(JIBAL_DATADIR "${PROJECT_SOURCE_DIR}/data")
    set(JIBAL_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}")
else()
    set(JIBAL_DATADIR "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/")
    set(JIBAL_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
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
FILE(GLOB PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/jibal*.h")
set(PUBLIC_HEADERS "${PUBLIC_HEADERS};${CMAKE_CURRENT_BINARY_DIR}/jibal_config.h;")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS)

add_library(Jibal SHARED
        gsto.c
        masses.c
        phys.c
        units.c
        material.c
        layer.c
        kin.c
        cross_section.c
        jibal.c
        stragg.c
        stop.c
        win_compat.c
        )


target_link_libraries(Jibal GSL::gsl) #GSL is found by FindGSL.

target_include_directories(Jibal
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
        )

set_property(TARGET Jibal PROPERTY VERSION ${jibal_VERSION})
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
        VERSION "${jibal_VERSION_MAJOR}.${jibal_VERSION_MINOR}"
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
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION "${jibal_VERSION_MAJOR}.${jibal_VERSION_MINOR}.${jibal_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR "${jibal_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${jibal_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${jibal_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "jibal-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES
        "^${CMAKE_SOURCE_DIR}/.git/"
        "^${CMAKE_SOURCE_DIR}/\.idea/"
        ".gitignore$"
        ".DS_Store$"
        "^${CMAKE_SOURCE_DIR}/build/"
        "^${CMAKE_SOURCE_DIR}/cmake-build-.*/"
        "^${CMAKE_SOURCE_DIR}/gui/build-.*/"
        )
include(CPack)
