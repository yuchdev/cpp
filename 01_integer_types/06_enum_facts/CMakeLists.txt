project(06_enum_facts CXX)
set(TARGET 06_enum_facts)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "IntegerTypes")
