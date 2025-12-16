set(TARGET 03_static_init_and_main)

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
