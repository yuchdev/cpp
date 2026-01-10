set(TARGET 05_c_things_not_in_cpp)

file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY LINKER_LANGUAGE C)
set_property(TARGET ${TARGET} PROPERTY FOLDER "13CCpp")
