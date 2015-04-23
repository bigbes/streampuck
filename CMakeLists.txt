cmake_minimum_required(VERSION 2.6)

project(streampuck)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING
        "Choose the type of build, options are: Debug, Release" FORCE)
endif()

set(SPLIB_VERSION_MAJOR "0")
set(SPLIB_VERSION_MINOR "1")

set(SPLIB_VERSION   "${SPLIB_VERSION_MAJOR}.${SPLIB_VERSION_MINOR}")
set(SPLIB_SOVERSION "${SPLIB_VERSION_MAJOR}")


set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -march=native")
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O0 -g3 -ggdb3")
endif()

include_directories("${PROJECT_SOURCE_DIR}/msgpuck")

message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

set(sp_lib_sources
        sp.c
        sp_mem.c
        mp_custom.c
)

add_library(sp SHARED ${sp_lib_sources})
set_target_properties(sp PROPERTIES COMPILE_FLAGS "${CMAKE_C_FLAGS}")
set_target_properties(sp PROPERTIES VERSION ${SPLIB_VERSION} SOVERSION ${SPLIB_SOVERSION})

add_subdirectory("test")
