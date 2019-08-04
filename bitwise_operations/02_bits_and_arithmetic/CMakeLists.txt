cmake_minimum_required(VERSION 3.0)
project(02_bits_and_arithmetic CXX)
set(TARGET 02_bits_and_arithmetic)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
