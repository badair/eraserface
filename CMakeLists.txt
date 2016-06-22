
# Copyright Louis Dionne 2015
# Modified Work Copyright Barrett Adair 2015
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.0)
project(eraserface CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
enable_testing()

# Perform checks to make sure we support the current compiler
include(CheckCxxCompilerSupport)

if(BUILD_CXX_STD)
else()
  # Defaults to C++14 if not set:
  set(BUILD_CXX_STD 14) 
endif()

set (CMAKE_CXX_STANDARD ${eraserface_CXX_STD})

# Setting up CMake options and compiler flags (more flags can be set on a per-target basis or in subdirectories)

include(CheckCXXCompilerFlag)
macro(eraserface_append_flag testname flag)
    check_cxx_compiler_flag(${flag} ${testname})
    if (${testname})
        add_compile_options(${flag})
    endif()
endmacro()


eraserface_append_flag(eraserface_HAS_WERROR -Werror)
eraserface_append_flag(eraserface_HAS_WX -WX)
eraserface_append_flag(eraserface_HAS_QUNUSED_ARGUMENTS          -Qunused-arguments)
eraserface_append_flag(eraserface_HAS_WNO_UNUSED_LOCAL_TYPEDEFS  -Wno-unused-local-typedefs)
eraserface_append_flag(eraserface_HAS_WWRITE_STRINGS             -Wwrite-strings)

if(NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    eraserface_append_flag(eraserface_HAS_FGNU_TM                    -fgnu-tm)
endif()

if(MSVC AND NOT ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    #disable warning about symbol truncation. Doesn't matter, affected types are not linked
    eraserface_append_flag(eraserface_HAS_WD4503                     -wd4503)
    eraserface_append_flag(eraserface_HAS_W3                         -W3)
else()
    eraserface_append_flag(eraserface_HAS_W                          -W)
    eraserface_append_flag(eraserface_HAS_WALL                       -Wall)
    eraserface_append_flag(eraserface_HAS_WEXTRA                     -Wextra)
endif()

if(NOT MSVC)
    eraserface_append_flag(eraserface_HAS_FTEMPLATE_BACKTRACE_LIMIT  -ftemplate-backtrace-limit=0)
    eraserface_append_flag(eraserface_HAS_PEDANTIC                   -pedantic)
    eraserface_append_flag(eraserface_HAS_STDCXX1Y                   -std=c++1y)
endif()

find_package(Boost 1.59)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else()
    message(ERROR
        "The Boost library headers were not found; tests will not be built.")
endif()

include_directories(../callable_traits/include)

##############################################################################
# eraserface_target_name_for(<output variable> <source file> [ext])
# Returns the target name associated to a source file. If the path of the
# source file relative from the root of eraserface is `path/to/source/file.ext`,
# the target name associated to it will be `path.to.source.file`.
#
# The extension of the file should be specified as a last argument. If no
# extension is specified, the `.cpp` extension is assumed.
##############################################################################

function(eraserface_target_name_for out file)
    if (NOT ARGV2)
        set(_extension ".cpp")
    else()
        set(_extension "${ARGV2}")
    endif()

    file(RELATIVE_PATH _relative ${eraserface_SOURCE_DIR} ${file})
    string(REPLACE "${_extension}" "" _name ${_relative})
    string(REGEX REPLACE "/" "." _name ${_name})
    set(${out} "${_name}" PARENT_SCOPE)
endfunction()

##############################################################################
# eraserface_add_test(<name> <command> [<arg>...])
# Creates a test called `name`, which runs the given `command` with the given args.
##############################################################################

function(eraserface_add_test name)
    if (eraserface_ENABLE_MEMCHECK)
        add_test(${name} ${Valgrind_EXECUTABLE} --leak-check=full --error-exitcode=1 ${ARGN})
    else()
        add_test(${name} ${ARGN})
    endif()
endfunction()

##############################################################################
# Setup the `check` target to build and then run all the tests and examples.
##############################################################################


add_custom_target(check
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Build and then run all the tests and examples.")

add_subdirectory(example)
add_subdirectory(test)

##############################################################################
# Setup the 'install' target.
# This copies the whole content of include/ to ${CMAKE_INSTALL_PREFIX}.
##############################################################################

install(DIRECTORY include/boost DESTINATION include
        PATTERN ".DS_Store" EXCLUDE)
