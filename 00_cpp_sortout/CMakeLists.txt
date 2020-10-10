project(cpp_sortout CXX)
set(TARGET cpp_sortout)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "CppSortout")

target_link_libraries(${TARGET}    
PRIVATE
    utilities
)
