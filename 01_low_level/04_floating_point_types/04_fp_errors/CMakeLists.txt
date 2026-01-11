project(05_fp_errors CXX)
set(TARGET 05_fp_errors)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "02_FpTypes")

# MSVC-only compatibility defines (avoid polluting non-MSVC builds)
target_compile_definitions(${TARGET}
PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:_USE_MATH_DEFINES>
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
