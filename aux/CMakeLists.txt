add_executable(get_stop get_stop.c)
add_executable(get_kin get_kin.c)
add_executable(jibaltool jibaltool.c)
target_link_libraries(get_stop Jibal)
target_link_libraries(get_kin Jibal)
target_include_directories(jibaltool PRIVATE
        $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/jibal>)
target_link_libraries(jibaltool Jibal)
install(TARGETS get_stop get_kin jibaltool RUNTIME DESTINATION bin)
