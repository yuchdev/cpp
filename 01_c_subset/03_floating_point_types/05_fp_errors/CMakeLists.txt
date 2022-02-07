project(05_fp_errors CXX)
set(TARGET 05_fp_errors)

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
