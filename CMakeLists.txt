cmake_minimum_required (VERSION 3.9)

# set(CMAKE_C_COMPILER "/usr/local/bin/gcc")
# set(CMAKE_CXX_COMPILER "/usr/local/bin/g++")

PROJECT (main)

include(utils.cmake)

# 打开C++11标准, 需设置编译选项开启c++11
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb -fno-common")

MESSAGE(STATUS "In main project This is BINARY dir " ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "In main project This is SOURCE dir " ${PROJECT_SOURCE_DIR})

add_definitions(-DTHREAD_MEASURE)
add_definitions(-DMEM_CHECK)

add_definitions(-Wno-builtin-macro-redefined)

ADD_SUBDIRECTORY(base)
ADD_SUBDIRECTORY(dpdk_adapt)
ADD_SUBDIRECTORY(test)
