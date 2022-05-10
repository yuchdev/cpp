set(TARGET typeid_rtti)

file(GLOB SOURCES *.cpp *.h *.txt, *.md)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "05Inheritance2")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)

