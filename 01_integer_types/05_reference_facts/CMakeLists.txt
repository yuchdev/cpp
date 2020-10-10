project(05_reference_facts CXX)
set(TARGET 05_reference_facts)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "IntegerTypes")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)
