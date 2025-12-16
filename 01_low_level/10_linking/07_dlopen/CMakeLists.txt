set(TARGET 07_dlopen)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "10Linking")

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
