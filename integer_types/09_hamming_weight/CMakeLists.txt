cmake_minimum_required(VERSION 3.0)
project(08_hamming_weight CXX)
set(TARGET 08_hamming_weight)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
