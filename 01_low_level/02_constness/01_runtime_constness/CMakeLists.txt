set(TARGET 01_runtime_constness)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "00_FundamentalTypes")

# Suppress warning for returning address/reference to local temporary object for this example only
# Clang uses -Wreturn-stack-address, GCC uses -Wreturn-local-addr, MSVC uses C4172
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${TARGET} PRIVATE -Wno-return-stack-address)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${TARGET} PRIVATE -Wno-return-local-addr)
elseif (MSVC)
    target_compile_options(${TARGET} PRIVATE /wd4172)
endif()

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
