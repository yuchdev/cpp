set(TARGET class_basics)

file(GLOB SOURCES *.cpp *.h *.txt, *.md)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "01Classes")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)

