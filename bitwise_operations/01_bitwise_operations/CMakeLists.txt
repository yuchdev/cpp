cmake_minimum_required(VERSION 3.0)
project(01_bitwise_operations CXX)
set(TARGET 01_bitwise_operations)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
