set(TARGET 04_attributes_and_preprocessor)

file(GLOB SOURCES *.cpp *.h)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

add_executable(${TARGET} ${SOURCES})
set_property(TARGET ${TARGET} PROPERTY FOLDER "09Functions")

# Disable deprecation warnings for this specific target (educational content)
if (MSVC)
    target_compile_options(${TARGET} PRIVATE /wd4996)
else()
    target_compile_options(${TARGET} PRIVATE -Wno-deprecated-declarations)
endif()

target_link_libraries(${TARGET}
PRIVATE
    utilities
)
