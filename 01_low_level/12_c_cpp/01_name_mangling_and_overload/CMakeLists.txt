set(TARGET 01_name_mangling_and_overload)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "13CCpp")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
