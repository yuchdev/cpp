set(TARGET align)

file(GLOB SOURCES *.cpp *.h *.txt, *.md)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "02Memory")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)

