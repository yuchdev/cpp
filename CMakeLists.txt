cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(cpp.h.in version.h)

# This is name and version of your project
set(PROJECT_NAME cpp)
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

if(MSVC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /FC /EHa /W3 /MP")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /ZI")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi /Oi /GL /Gy")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")

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

add_subdirectory(utilities)
add_subdirectory(cpp_sortout)
add_subdirectory(01_c_subset)
add_subdirectory(02_class_subset)
#add_subdirectory(03_template_subset)
#add_subdirectory(04_stl_subset)
#add_subdirectory(05_multithread_subset)

