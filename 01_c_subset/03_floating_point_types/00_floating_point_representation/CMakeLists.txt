project(01_floating_point_representation CXX)
set(TARGET 01_floating_point_representation)

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
