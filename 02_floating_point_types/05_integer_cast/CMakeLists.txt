project(05_integer_cast CXX)
set(TARGET 05_integer_cast)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "FpTypes")
