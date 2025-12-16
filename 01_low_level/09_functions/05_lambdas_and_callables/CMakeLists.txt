set(TARGET 05_lambdas_and_callables)

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
