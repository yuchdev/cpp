set(TARGET 02_arrays_templates_and_traits)

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
