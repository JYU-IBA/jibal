add_executable(get_stop get_stop.c)
add_executable(get_kin get_kin.c)
if(WIN32)
#find_package(getopt)
find_path(GETOPT_INCLUDE_DIR getopt.h)
find_library(GETOPT_LIBRARY getopt)
endif()
add_executable(jibaltool jibaltool.c)
add_executable(jibal_bootstrap jibal_bootstrap.c)
add_executable(srim_gen_stop srim_gen_stop.c)
add_executable(dpass_decode dpass_decode.c)
target_include_directories(get_stop PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/jibal>
        ${GETOPT_INCLUDE_DIR}
        )
target_link_libraries(get_stop
    PRIVATE Jibal
    "$<$<BOOL:${UNIX}>:m>"
    ${GETOPT_LIBRARY}
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

target_include_directories(jibal_bootstrap PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/jibal>
        )
target_link_libraries(jibal_bootstrap
        PRIVATE Jibal
        )

target_link_libraries(srim_gen_stop
    PRIVATE Jibal
    "$<$<BOOL:${UNIX}>:m>"
)
install(TARGETS get_stop get_kin jibaltool srim_gen_stop dpass_decode jibal_bootstrap
        RUNTIME DESTINATION bin
        COMPONENT applications)

if(WIN32) #Extra DLLs for Windows installation that are not handled by InstallRequiredSystemLibraries
    set(DLL_ROOT "${_VCPKG_ROOT_DIR}/installed/${VCPKG_TARGET_TRIPLET}/bin")
    message(STATUS "Trying to install extra DLLs from ${DLL_ROOT}")
    install(FILES "${DLL_ROOT}/getopt.dll"
    DESTINATION bin
    COMPONENT applications)
endif()
