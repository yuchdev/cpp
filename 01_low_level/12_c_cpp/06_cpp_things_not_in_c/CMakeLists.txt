set(TARGET 06_cpp_things_not_in_c)

file(GLOB SOURCES *.cpp)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "13CCpp")
