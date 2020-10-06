cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(Cpp.h.in Cpp.h)

# This is name and version of your project
set(PROJECT_NAME Cpp)
project(
  ${PROJECT_NAME}
  VERSION 0.1
  LANGUAGES CXX
)


# This is in-source build guard
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/utilities
)


if(MSVC)
  add_compile_options(PUBLIC "/W3 /WX /EHa /FC /MP" 
    "$<$<CONFIG:Release>:/Zi /Oi /GL /Gy /O2>" 
    "$<$<CONFIG:Debug>:/ZI>"
  )
  message("Windows MSVC++ configuration")

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

    # define WIN32 macro version
    macro(get_win32_winnt version)
        if (WIN32 AND CMAKE_SYSTEM_VERSION)
            set(ver ${CMAKE_SYSTEM_VERSION})
            string(REPLACE "." "" ver ${ver})
            string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

            set(${version} "0x${ver}")
        endif()
    endmacro()

  message("Set WINVER: " ver)
  message("Set SCL_SECURE_NO_WARNINGS")
  get_win32_winnt(ver)
  add_definitions(-D_WIN32_WINNT=${ver})
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)

endif()

add_subdirectory(00_cpp_sortout)
add_subdirectory(01_integer_types)
add_subdirectory(02_floating_point_types)

