cmake_minimum_required(VERSION 3.0)
project(07_unexpected_cast CXX)
set(TARGET 07_unexpected_cast)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
