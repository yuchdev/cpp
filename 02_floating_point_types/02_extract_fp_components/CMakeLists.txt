cmake_minimum_required(VERSION 3.0)
project(02_extract_fp_components CXX)
set(TARGET 02_extract_fp_components)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
