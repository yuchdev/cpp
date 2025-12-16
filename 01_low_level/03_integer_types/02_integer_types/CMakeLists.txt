set(TARGET 02_integer_types)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "03IntegerTypes")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
