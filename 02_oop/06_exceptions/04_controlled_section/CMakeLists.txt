set(TARGET 04_controlled_section)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "06Exceptions")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)

