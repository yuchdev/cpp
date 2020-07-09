cmake_minimum_required(VERSION 3.0)
project(05_integer_cast CXX)
set(TARGET 05_integer_cast)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
