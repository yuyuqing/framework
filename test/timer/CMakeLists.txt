PROJECT (timer)

MESSAGE(STATUS "in timer project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "in timer project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../../pub)

set(base_incs
    ${PROJECT_SOURCE_DIR}/../../base/include)

include_directories(${pub_incs} 
                    ${base_incs})

set(lib_obj
    sub::libbase)


set(main_timer_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/main_timer.cpp)
add_executable(main_timer ${main_timer_srcs})
redefine_file_macro(main_timer)
target_link_libraries(main_timer -lglib-2.0 -Wl,--whole-archive ${lib_obj} -Wl,--no-whole-archive -lrt -pthread)
