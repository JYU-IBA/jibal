add_executable(get_stop get_stop.c)
add_executable(get_kin get_kin.c)
if(WIN32)
#find_package(getopt)
find_path(GETOPT_INCLUDE_DIR getopt.h)
find_library(GETOPT_LIBRARY getopt)
endif()
add_executable(jibaltool jibaltool.c)
add_executable(srim_gen_stop srim_gen_stop.c)
add_executable(dpass_decode dpass_decode.c)
target_link_libraries(get_stop
    PRIVATE Jibal
    "$<$<BOOL:${UNIX}>:m>"
)
target_link_libraries(get_kin
    PRIVATE Jibal
    "$<$<BOOL:${UNIX}>:m>"
)
target_link_libraries(dpass_decode
    PRIVATE Jibal
    "$<$<BOOL:${UNIX}>:m>"
)
target_include_directories(jibaltool PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/jibal>
        ${GETOPT_INCLUDE_DIR}
)
target_link_libraries(jibaltool
    PRIVATE Jibal
    "$<$<BOOL:${UNIX}>:m>"
    ${GETOPT_LIBRARY})
target_link_libraries(srim_gen_stop
    PRIVATE Jibal
    "$<$<BOOL:${UNIX}>:m>"
)
install(TARGETS get_stop get_kin jibaltool srim_gen_stop dpass_decode
        RUNTIME DESTINATION bin
        COMPONENT applications)



if(WIN32) #Extra DLLs for Windows installation that are not handled by InstallRequiredSystemLibraries
    set(DLL_ROOT "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}/bin")
    message(STATUS "Trying to install extra DLLs from ${DLL_ROOT}")
    install(FILES "${DLL_ROOT}/getopt.dll"
    DESTINATION bin
    COMPONENT applications)
endif()

if(JIBAL_BUNDLE) #Almost could work
if(WIN32)
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
include(InstallRequiredSystemLibraries)
set(APPS  "\${CMAKE_INSTALL_PREFIX}/bin/jibaltool.exe")
set(DIRS "\$ENV{VCPKG_ROOT}/installed/x64-windows/bin") #TODO: if(ARCH_X64) ... or something more clever

install(CODE "
   include(BundleUtilities)
   fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
   " COMPONENT runtime)
endif()
endif()
