project(08_hamming_weight CXX)
set(TARGET 08_hamming_weight)

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
