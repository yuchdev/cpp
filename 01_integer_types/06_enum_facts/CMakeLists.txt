cmake_minimum_required(VERSION 3.0)
project(06_enum_facts CXX)
set(TARGET 06_enum_facts)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
