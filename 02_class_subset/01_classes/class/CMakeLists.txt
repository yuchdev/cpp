set(TARGET class)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "00_FundamentalTypes")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)
