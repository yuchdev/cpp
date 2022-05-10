set(TARGET virtual)

file(GLOB SOURCES *.cpp *.h *.txt, *.md)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "04Inheritance1")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)

