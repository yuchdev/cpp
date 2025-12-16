set(TARGET 02_extern_c_callbacks)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "13CCpp")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
