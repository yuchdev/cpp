set(TARGET 03_function_pointers)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "09Functions")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
