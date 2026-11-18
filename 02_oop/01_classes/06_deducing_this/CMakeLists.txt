set(TARGET 06_deducing_this)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "01Classes")

# Explicit object parameters ("deducing this") are a C++23 language
# feature; this one target opts into C++23 rather than raising the whole
# project's C++20 standard.
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD_REQUIRED ON)

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
