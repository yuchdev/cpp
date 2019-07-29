cmake_minimum_required(VERSION 2.8)
project(bitwise_operations CXX)
set(TARGET bitwise_operations)

########################################################
# source files
# source files
file(GLOB SOURCES *.cpp *.h *.txt)

########################################################
# include directories
set(INCLUDE_DIRS ${INCLUDE_DIRS} . )
include_directories(${INCLUDE_DIRS})

########################################################
add_executable(${TARGET} ${SOURCES})
