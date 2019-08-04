cmake_minimum_required(VERSION 2.8)
project(cpp CXX)
set(TARGET cpp)

if( ("UNIX") AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") )

    # using Clang
    message("UNIX congiguration, Clang")
    set(CMAKE_CXX_FLAGS "-m32 -std=c++11")
    set(CMAKE_EXE_LINKER_FLAGS "-std=c++11  -Wl")
elseif( ("UNIX") AND ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") )

    # using GCC
    message("UNIX congiguration, GCC")
    message("WARNING: gcc-multilib is required on x64 systems for the cross-compilation!")
    set(CMAKE_CXX_FLAGS "-m32 -std=c++11 ")
    set(CMAKE_EXE_LINKER_FLAGS "-std=c++11  -Wl,--no-as-needed")

elseif("WIN32")

  message("Windows configuration")
  set(CMAKE_CXX_FLAGS "/EHa")

endif()

########################################################

add_subdirectory(bitwise_operations)
