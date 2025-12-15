set(TARGET 03_enums_flags_and_operators)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "01_IntegerTypes")

# Suppress warning for sizeof on array function parameter for this example only
# Clang: -Wsizeof-array-argument; GCC typically doesn't warn here; MSVC has no direct equivalent
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET} PRIVATE -Wno-sizeof-array-argument)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # no-op: no direct GCC equivalent
elseif (MSVC)
    # no-op: no direct MSVC equivalent warning
endif()

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
