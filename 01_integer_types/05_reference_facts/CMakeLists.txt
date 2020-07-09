cmake_minimum_required(VERSION 3.0)
project(05_reference_facts CXX)
set(TARGET 05_reference_facts)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
