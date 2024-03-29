if(WIN32)
#find_package(getopt)
find_path(GETOPT_INCLUDE_DIR getopt.h)
find_library(GETOPT_LIBRARY getopt)
endif()
add_executable(jibaltool jibaltool.c jibaltool_get_stop.c jibaltool_get_stop.h)
add_executable(jibal_bootstrap jibal_bootstrap.c)
add_executable(srim_gen_stop srim_gen_stop.c)
add_executable(dpass_decode dpass_decode.c)

target_link_libraries(dpass_decode
    PRIVATE jibal
    "$<$<BOOL:${UNIX}>:m>"
)
target_include_directories(jibaltool PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/jibal>
        ${GETOPT_INCLUDE_DIR}
)
target_link_libraries(jibaltool
    PRIVATE jibal
    "$<$<BOOL:${UNIX}>:m>"
    ${GETOPT_LIBRARY})

target_include_directories(jibal_bootstrap PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/jibal>
        )
target_link_libraries(jibal_bootstrap
        PRIVATE jibal
        "$<$<BOOL:${UNIX}>:m>"
        )

target_link_libraries(srim_gen_stop
    PRIVATE jibal
    "$<$<BOOL:${UNIX}>:m>"
)
install(TARGETS jibaltool srim_gen_stop dpass_decode jibal_bootstrap
        RUNTIME DESTINATION bin
        COMPONENT applications)

if(WIN32) #Extra DLLs for Windows installation that are not handled by InstallRequiredSystemLibraries
    if(_VCPKG_INSTALLED_DIR)
        set(DLL_ROOT "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
        message(STATUS "Trying to install extra DLLs (getopt.dll) from ${DLL_ROOT}")
    else()
        set(DLL_ROOT ".")
        message(STATUS "Trying to install extra DLLs (getopt.dll) from this directory.")
    endif()
    install(FILES "${DLL_ROOT}/getopt.dll"
    DESTINATION bin
    COMPONENT applications)
endif()
