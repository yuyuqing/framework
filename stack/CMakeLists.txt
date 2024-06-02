PROJECT (stack)

MESSAGE(STATUS "In stack project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "In stack project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(dpdk_incs
    /home/wrk1/dpdk_install/include)

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../pub)

set(base_incs
    ${PROJECT_SOURCE_DIR}/../base/include)

set(stack_incs
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${dpdk_incs} 
                    ${pub_incs} 
                    ${base_incs} 
                    ${stack_incs})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source stack_srcs)

ADD_LIBRARY(${PROJECT_NAME} ${stack_srcs})
redefine_file_macro(${PROJECT_NAME})
ADD_LIBRARY(sub::libstack ALIAS ${PROJECT_NAME})