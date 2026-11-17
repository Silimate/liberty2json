###############################################################################
# Project setup
###############################################################################

cmake_minimum_required(VERSION 3.16) # match OpenSTA
project(Liberty2JSON C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Detect OS
if (UNIX)
  if (APPLE)
    set(OS mac)
  else()
    set(OS linux)
  endif()
endif()

# Fall back to RELEASE when no build type is given
if (NOT CMAKE_BUILD_TYPE)
  message(WARNING "Build type not set, falling back to RELEASE mode.
 To specify build type use:
 -DCMAKE_BUILD_TYPE=<mode> where <mode> is DEBUG or RELEASE.")
  set(CMAKE_BUILD_TYPE "RELEASE" CACHE STRING "Choose the type of build, options are: DEBUG RELEASE." FORCE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG
"${CMAKE_CXX_FLAGS_DEBUG} -Wall -O0 -g"
)
set(CMAKE_CXX_FLAGS_RELEASE
"${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3"
)

set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}})

# ipa icf pass segfaults in GCC 11
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "12.0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-ipa-icf")
endif()

###############################################################################
# Libraries
###############################################################################

if (UNIX AND NOT APPLE)
  set (STACK_DETAILS_AUTO_DETECT FALSE CACHE BOOL "Auto detect backward's stack details dependencies")
  set (STACK_DETAILS_DW TRUE CACHE BOOL "Use libdw to read debug info")
endif()
add_subdirectory(third_party/backward-cpp)
target_include_directories(backward_interface INTERFACE /usr/include/libdwarf/)
add_subdirectory(third_party/opensta)
set(JSONCONS_BUILD_TESTS OFF)
add_subdirectory(third_party/jsoncons)

###############################################################################
# Add the 'src' subdirectory
###############################################################################

add_subdirectory(src)
