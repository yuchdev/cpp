cmake_minimum_required(VERSION 2.8)
project(integer_types CXX)
set(TARGET integer_types)

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
