PROJECT (log)

MESSAGE(STATUS "in log project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "in log project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../../pub)

set(base_incs
    ${PROJECT_SOURCE_DIR}/../../base/include)

include_directories(${pub_incs} 
                    ${base_incs})

set(lib_obj
    sub::libbase)


set(main_log_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/main_log.cpp)
add_executable(main_log ${main_log_srcs})
redefine_file_macro(main_log)
target_link_libraries(main_log -lglib-2.0 -Wl,--whole-archive ${lib_obj} -Wl,--no-whole-archive -lrt -pthread)
