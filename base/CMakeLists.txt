PROJECT (base)

MESSAGE(STATUS "In base project This is SOURCE  dir " ${PROJECT_SOURCE_DIR})
MESSAGE(STATUS "In base project This is CURRENT dir " ${CMAKE_CURRENT_SOURCE_DIR})

set(pub_incs
    ${PROJECT_SOURCE_DIR}/../pub)

set(base_incs
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${pub_incs} ${base_incs})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source base_srcs)

ADD_LIBRARY(${PROJECT_NAME} ${base_srcs})
redefine_file_macro(${PROJECT_NAME})
ADD_LIBRARY(sub::libbase ALIAS ${PROJECT_NAME})