set(TARGET 02_operator_new_delete)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "02Memory")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)

