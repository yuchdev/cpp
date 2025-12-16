set(TARGET 05_wrapping_c)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "11Namespaces")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
