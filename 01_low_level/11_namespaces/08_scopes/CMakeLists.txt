set(TARGET 12_scopes)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "03_Functions")

# Suppress warning about dangling else for this example only
# Clang/GCC: -Wno-dangling-else; MSVC has no direct equivalent
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET} PRIVATE -Wno-dangling-else)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${TARGET} PRIVATE -Wno-dangling-else)
elseif (MSVC)
    # no-op: no direct MSVC equivalent
endif()

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
