set(TARGET 07_posix_signals)

file(GLOB SOURCES *.cpp *.h *.txt, *.md)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "06Exceptions")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)
