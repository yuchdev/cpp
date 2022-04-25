set(TARGET utilities)

add_library(${TARGET} INTERFACE)
target_sources(${TARGET}
INTERFACE
        ${CMAKE_SOURCE_DIR}/utilities/bitwise.h
        ${CMAKE_SOURCE_DIR}/utilities/elapsed.h
        ${CMAKE_SOURCE_DIR}/utilities/generate.h
)

target_include_directories(${TARGET} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_property(TARGET ${TARGET} PROPERTY FOLDER "Utilities")