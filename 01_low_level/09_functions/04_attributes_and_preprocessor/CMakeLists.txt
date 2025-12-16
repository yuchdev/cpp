set(TARGET 04_attributes_and_preprocessor)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "09Functions")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
