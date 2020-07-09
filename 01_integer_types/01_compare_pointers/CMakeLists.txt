cmake_minimum_required(VERSION 3.0)
project(01_compare_pointers CXX)
set(TARGET 01_compare_pointers)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
