cmake_minimum_required(VERSION 3.0)
project(cpp_tricks CXX)
set(TARGET cpp_tricks)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})

