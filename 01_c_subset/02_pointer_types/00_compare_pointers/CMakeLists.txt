set(TARGET 00_compare_pointers)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "01_IntegerTypes")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)
