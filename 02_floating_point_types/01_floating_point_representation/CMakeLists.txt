cmake_minimum_required(VERSION 3.0)
project(01_floating_point_representation CXX)
set(TARGET 01_floating_point_representation)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
