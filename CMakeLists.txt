###############################################################################
# Project setup
###############################################################################

cmake_minimum_required(VERSION 3.5)
project(Liberty2JSON C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Detect build type, fallback to release and throw a warning if use didn't
# specify any
if(NOT CMAKE_BUILD_TYPE)
  message(WARNING "Build type not set, falling back to RELEASE mode.
 To specify build type use:
 -DCMAKE_BUILD_TYPE=<mode> where <mode> is DEBUG or RELEASE.")
  set(CMAKE_BUILD_TYPE
      "RELEASE"
      CACHE STRING "Choose the type of build, options are: DEBUG RELEASE."
            FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_CXX_FLAGS_DEBUG
"${CMAKE_CXX_FLAGS_DEBUG} -Wall -O3 -g"
)
set(CMAKE_CXX_FLAGS_RELEASE
"${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3"
)

set (CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}})

###############################################################################
# Fetch files
###############################################################################

file(GLOB LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c)
list(REMOVE_ITEM LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/syntform.c)
file(GLOB LIB_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)

###############################################################################
# Flex/Bison targets
###############################################################################

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)
bison_target(liberty_parser ${CMAKE_CURRENT_SOURCE_DIR}/src/liberty_parser.y ${CMAKE_CURRENT_SOURCE_DIR}/src/liberty_parser.c)
bison_target(syntax_parser ${CMAKE_CURRENT_SOURCE_DIR}/src/syntax_parser.y ${CMAKE_CURRENT_SOURCE_DIR}/src/syntax_parser.c)
flex_target(token ${CMAKE_CURRENT_SOURCE_DIR}/src/token.l ${CMAKE_CURRENT_SOURCE_DIR}/src/token.c)
flex_target(synttok ${CMAKE_CURRENT_SOURCE_DIR}/src/synttok.l ${CMAKE_CURRENT_SOURCE_DIR}/src/synttok.c)
add_flex_bison_dependency(token liberty_parser)
add_flex_bison_dependency(synttok syntax_parser)
set(FLEX_BISON_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/liberty_parser.c ${CMAKE_CURRENT_SOURCE_DIR}/src/syntax_parser.c ${CMAKE_CURRENT_SOURCE_DIR}/src/synttok.c ${CMAKE_CURRENT_SOURCE_DIR}/src/token.c)


###############################################################################
# Build Verific
###############################################################################

add_custom_target(BuildVerificLiberty DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/synlib/synlib-linux.a)
add_custom_command(
  OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/synlib/synlib-linux.a
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/synlib/synlib_file.h
  COMMAND echo "       Build Verific Liberty parser library"
  COMMAND make CFLAGS="-DVERIFIC_ENABLE_ZLIB=1" VERSION=-O3
  COMMAND echo "       Verific Liberty parser library"
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/synlib/")

add_custom_target(BuildVerificDatabase DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/database/database-linux.a)
add_custom_command(
    OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/database/database-linux.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/database/DataBase.h
    COMMAND echo "       Build Verific database library"
    COMMAND make CFLAGS="-DVERIFIC_ENABLE_ZLIB=1" VERSION=-O3
    COMMAND echo "       Verific database library"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/database/")


add_custom_target(BuildVerificContainers DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/containers/containers-linux.a)
add_custom_command(
    OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/containers/containers-linux.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/containers/Array.h
    COMMAND echo "       Build Verific containers library"
    COMMAND make CFLAGS="-DVERIFIC_ENABLE_ZLIB=1" VERSION=-O3
    COMMAND echo "       Verific containers library"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/containers/")


add_custom_target(BuildVerificUtil DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/util/util-linux.a)
add_custom_command(
      OUTPUT  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/util/util-linux.a
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/util/VerificSystem.h
      COMMAND echo "       Build Verific util library"
      COMMAND make CFLAGS="-DVERIFIC_ENABLE_ZLIB=1" VERSION=-O3
      COMMAND echo "       Verific util library"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/util/")

###############################################################################
# Subsystem
###############################################################################
set (STACK_DETAILS_AUTO_DETECT FALSE CACHE BOOL "Auto detect backward's stack details dependencies")
set (STACK_DETAILS_DW TRUE CACHE BOOL "Use libdw to read debug info")
add_subdirectory(third_party/backward-cpp)
target_include_directories(backward_interface INTERFACE /usr/include/libdwarf/)

###############################################################################
# Executable target for Liberty2JSON
###############################################################################

add_executable(liberty2json ${CMAKE_CURRENT_SOURCE_DIR}/liberty2json.cpp 
                            ${PROJECT_SOURCE_DIR}/third_party/backward-cpp/backward.cpp
                            ${PROJECT_SOURCE_DIR}/VerificLibertyParser.cpp
                            ${PROJECT_SOURCE_DIR}/JsonVisitor.cpp
                            ${LIB_SOURCES} ${LIB_HEADERS} ${FLEX_BISON_SOURCES})
target_include_directories(liberty2json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/synlib/)
target_include_directories(liberty2json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/util/)
target_include_directories(liberty2json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/containers/)
target_include_directories(liberty2json PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
                                               ${PROJECT_SOURCE_DIR}/third_party)  
target_link_libraries(liberty2json PUBLIC Backward::Interface)

add_library(synlib STATIC IMPORTED)
set_target_properties(synlib PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/synlib/synlib-linux.a)
add_dependencies(synlib BuildVerificLiberty)

add_library(containers STATIC IMPORTED)
set_target_properties(containers PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/containers/containers-linux.a)
add_dependencies(containers BuildVerificContainers)

add_library(database STATIC IMPORTED)
set_target_properties(database PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/database/database-linux.a)
add_dependencies(database BuildVerificDatabase)

add_library(util STATIC IMPORTED)
set_target_properties(util PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/verific/util/util-linux.a)
add_dependencies(util BuildVerificUtil)

# !!! The order of the libraries is very specific to avoid undefined symbols !!!
target_link_libraries(liberty2json PUBLIC synlib database containers util z)

###############################################################################
# Executable target for syntform
###############################################################################
add_executable(syntform ${CMAKE_CURRENT_SOURCE_DIR}/src/syntform.c ${LIB_SOURCES} ${LIB_HEADERS} ${FLEX_BISON_SOURCES})
target_include_directories(syntform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
