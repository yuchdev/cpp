set(TARGET 03_references)

file(GLOB SOURCES *.cpp *.h *.txt)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "01_IntegerTypes")

# Suppress warning for returning reference/address to local temporary object for this example only
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
