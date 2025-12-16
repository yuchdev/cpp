set(TARGET 04_popcount_and_scans)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "08BitwiseOps")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
