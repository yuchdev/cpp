project(07_fast_reverse_sqrt CXX)
set(TARGET 07_fast_reverse_sqrt)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "02_FpTypes")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)
