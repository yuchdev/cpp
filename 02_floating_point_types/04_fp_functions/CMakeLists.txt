cmake_minimum_required(VERSION 3.0)
project(04_fp_functions CXX)
set(TARGET 04_fp_functions)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
