PROJECT (packet)

MESSAGE(STATUS "in packet project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "in packet project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../../pub)

set(base_incs
    ${PROJECT_SOURCE_DIR}/../../base/include)

include_directories(${pub_incs} 
                    ${base_incs})

set(lib_obj
    sub::libbase)


set(main_packet_srcs
    ${CMAKE_CURRENT_SOURCE_DIR}/main_packet.cpp)
add_executable(main_packet ${main_packet_srcs})
redefine_file_macro(main_packet)
target_link_libraries(main_packet -lglib-2.0 ${lib_obj} -lrt -pthread)
