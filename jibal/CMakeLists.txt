#find_package(PkgConfig)

find_package(GSL)
#include(FindGSL) #GSL is a special snowflake.
if(NOT GSL_FOUND)
    message(FATAL_ERROR "GSL not found")
endif()

if(WIN32)
    set(PLATFORM_DEPS ${PLATFORM_DEPS} Pathcch.lib)
endif()

include(CheckSymbolExists)
set(CMAKE_REQUIRED_LIBRARIES "m")

set(CMAKE_C_STANDARD 99)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(INCLUDE_INSTALL_DIR include/jibal)
set(LIB_INSTALL_DIR lib/)

option(DEVELOPER_MODE_ENABLE "Enable developer mode" OFF)
if(DEVELOPER_MODE_ENABLE)
    set(JIBAL_DATADIR "data") #relative to install prefix
    set(JIBAL_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}")
else()
    set(JIBAL_DATADIR "share/jibal") #relative to install prefix
    set(JIBAL_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
endif()

configure_file(jibal_defaults.h.in jibal_defaults.h @ONLY)

set(PKGCONF_REQ_PUB "${PKGCONF_REQ_PUB} gsl") #Sets a variable for pkg-config file generation
configure_file(
        ${PROJECT_SOURCE_DIR}/jibal.pc.in
        ${PROJECT_BINARY_DIR}/jibal.pc
        @ONLY
)

FILE(GLOB DATAFILES "${CMAKE_SOURCE_DIR}/data/*.*")
list(REMOVE_ITEM DATAFILES "${CMAKE_SOURCE_DIR}/data/.gitignore")
#message(STATUS "${DATAFILES}")
FILE(GLOB PUBLIC_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/jibal*.h")
set(PUBLIC_HEADERS "${PUBLIC_HEADERS};${CMAKE_CURRENT_BINARY_DIR}/jibal_defaults.h;")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(jibal SHARED
        gsto.c
        masses.c
        phys.c
        units.c
        material.c
        layer.c
        kin.c
        cross_section.c
        jibal.c
        config.c
        stragg.c
        stop.c
        option.c
        cs.c
        registry.c
        generic.c
        r33.c
        "$<$<BOOL:${WIN32}>:win_compat.c>"
        )

target_link_libraries(jibal GSL::gsl ${PLATFORM_DEPS})

target_include_directories(jibal
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
        )

set_property(TARGET jibal PROPERTY VERSION ${Jibal_VERSION})
set_property(TARGET jibal PROPERTY SOVERSION 0)
set_property(TARGET jibal PROPERTY INTERFACE_jibal_MAJOR_VERSION 0)
set_property(TARGET jibal APPEND PROPERTY COMPATIBLE_INTERFACE_STRING Jibal_MAJOR_VERSION)

install(TARGETS jibal EXPORT JibalTargets
        RUNTIME DESTINATION bin COMPONENT runtime #This includes the DLL on Windows
        LIBRARY DESTINATION lib COMPONENT libraries
        ARCHIVE DESTINATION lib COMPONENT libraries)
# install headers, note there's a subdirectory
install(FILES ${PUBLIC_HEADERS}
        DESTINATION ${INCLUDE_INSTALL_DIR}
        COMPONENT headers
)

# install bundled data
install(FILES ${DATAFILES}
        DESTINATION ${JIBAL_DATADIR}
        COMPONENT data
)

# install pkg-config file
install(FILES ${CMAKE_BINARY_DIR}/jibal.pc
        DESTINATION lib/pkgconfig)

include(CMakePackageConfigHelpers)

# config file from a template, includes exports
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/JibalConfig.cmake"
        INSTALL_DESTINATION "lib/Jibal/cmake"
        PATH_VARS INCLUDE_INSTALL_DIR
#        NO_SET_AND_CHECK_MACRO
#        NO_CHECK_REQUIRED_COMPONENTS_MACRO
        )

# generate the version file for config
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/JibalConfigVersion.cmake"
        VERSION "${Jibal_VERSION_MAJOR}.${Jibal_VERSION_MINOR}.${Jibal_VERSION_PATCH}"
        COMPATIBILITY AnyNewerVersion
)
# install the targets to binary directory
export(EXPORT JibalTargets
        FILE "${CMAKE_CURRENT_BINARY_DIR}/JibalTargets.cmake"
        )
# install the configuration file to binary directory too so "installing" the library is not necessary

set(ConfigPackageLocation lib/Jibal/cmake)

install(EXPORT JibalTargets
  FILE JibalTargets.cmake
  DESTINATION ${ConfigPackageLocation}
  COMPONENT headers
)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/JibalConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/JibalConfigVersion.cmake
        DESTINATION ${ConfigPackageLocation}
        COMPONENT headers
        )
# cpack configuration
# applications are in "tools" directory
set(CPACK_PACKAGE_VENDOR "University of Jyvaskyla")
set(CPACK_PACKAGE_DESCRIPTION "Jyväskylä Ion Beam Analysis Library.")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/JYU-IBA/jibal/")
set(CPACK_PACKAGE_VERSION "${Jibal_VERSION_MAJOR}.${Jibal_VERSION_MINOR}.${Jibal_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR "${Jibal_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${Jibal_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${Jibal_VERSION_PATCH}")
set(CPACK_COMPONENTS_ALL applications runtime libraries headers data)
set(CMAKE_INSTALL_DEBUG_LIBRARIES TRUE)
set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT "runtime")
include(InstallRequiredSystemLibraries)
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "jibal-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES
        "^${CMAKE_SOURCE_DIR}/.git/"
        "^${CMAKE_SOURCE_DIR}/.idea/"
        ".gitignore$"
        ".DS_Store$"
        "^${CMAKE_SOURCE_DIR}/build/"
        "^${CMAKE_SOURCE_DIR}/cmake-build-.*/"
        "^${CMAKE_SOURCE_DIR}/gui/build-.*/"
        )
set(CPACK_PACKAGE_INSTALL_DIRECTORY "jibal")
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "JIBAL applications")
set(CPACK_COMPONENT_APPLICATIONS_DESCRIPTION "Tools to extract and use data from JIBAL")
#set(CPACK_COMPONENT_APPLICATIONS_GROUP "Runtime")
set(CPACK_COMPONENT_APPLICATIONS_DEPENDS data libraries runtime)
#set(CPACK_COMPONENT_GROUP_RUNTIME_DESCRIPTION "Tools and data to access some IBA relevant data.")

set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "Runtime files")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Files, e.g. DLLs, necessary to run JIBAL applications")

set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Data files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Bundled files, e.g. atomic data")
#set(CPACK_COMPONENT_DATA_GROUP "Runtime")

set(CPACK_COMPONENT_LIBRARIES_DISPLAY_NAME "Libraries")
set(CPACK_COMPONENT_LIBRARIES_DESCRIPTION "Libraries used to build programs with JIBAL")
#set(CPACK_COMPONENT_LIBRARIES_GROUP "Development")
#set(CPACK_COMPONENT_GROUP_DEVELOPMENT_DESCRIPTION "Things you need to make your own programs or use other programs using JIBAL.")

set(CPACK_COMPONENT_HEADERS_DISPLAY_NAME "C Headers")
set(CPACK_COMPONENT_HEADERS_DESCRIPTION "C header files for JIBAL.")
set(CPACK_COMPONENT_HEADERS_DEPENDS libraries)
#set(CPACK_COMPONENT_HEADERS_GROUP "Development")

if(WIN32)
    set(CPACK_PACKAGE_EXECUTABLES "jibal_bootstrap;JIBAL Bootstrap")
    set(CPACK_GENERATOR WIX)
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "JIBAL")
    set(CPACK_WIX_CMAKE_PACKAGE_REGISTRY ON)
    set(CPACK_WIX_UPGRADE_GUID 51FB2949-D386-4C24-B8F4-41B46A34B233)
    configure_file(wix_patch.xml.in ${CMAKE_CURRENT_BINARY_DIR}/wix_patch.xml)
    set(CPACK_WIX_PATCH_FILE ${CMAKE_CURRENT_BINARY_DIR}/wix_patch.xml)
endif()
include(CPack)
