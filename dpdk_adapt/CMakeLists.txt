PROJECT (dpdk_adapt)

MESSAGE(STATUS "In dpdk_adapt project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "In dpdk_adapt project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(dpdk_incs
    /home/wrk1/dpdk_install/include)

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../pub)

set(base_incs
    ${PROJECT_SOURCE_DIR}/../base/include)

set(dpdk_adapt_incs
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${dpdk_incs} 
                    ${pub_incs} 
                    ${base_incs} 
                    ${dpdk_adapt_incs})

add_compile_options(-mssse3)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source dpdk_adapt_srcs)

ADD_LIBRARY(${PROJECT_NAME} ${dpdk_adapt_srcs})
redefine_file_macro(${PROJECT_NAME})
ADD_LIBRARY(sub::libdpdk_adapt ALIAS ${PROJECT_NAME})