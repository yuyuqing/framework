PROJECT (share_memory)

MESSAGE(STATUS "in share_memory project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "in share_memory project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../../pub)

set(base_incs
    ${PROJECT_SOURCE_DIR}/../../base/include)

include_directories(${pub_incs} 
                    ${base_incs})

set(lib_obj
    sub::libbase)


set(main_primary_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/main_primary.cpp)
add_executable(main_primary ${main_primary_srcs})
redefine_file_macro(main_primary_srcs)
target_link_libraries(main_primary -lglib-2.0 ${lib_obj} -lrt -pthread)


set(main_secondary_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/main_secondary.cpp)
add_executable(main_secondary ${main_secondary_srcs})
redefine_file_macro(main_secondary_srcs)
target_link_libraries(main_secondary -lglib-2.0 ${lib_obj} -lrt -pthread)
