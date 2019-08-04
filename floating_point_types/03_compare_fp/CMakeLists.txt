cmake_minimum_required(VERSION 3.0)
project(03_compare_fp CXX)
set(TARGET 03_compare_fp)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
