cmake_minimum_required(VERSION 3.0)
project(04_array_and_pointer CXX)
set(TARGET 04_array_and_pointer)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
