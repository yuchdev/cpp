project(04_fp_functions CXX)
set(TARGET 04_fp_functions)

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
