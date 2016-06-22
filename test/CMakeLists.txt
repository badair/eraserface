
# Copyright Louis Dionne 2015
# Modified Work Copyright Barrett Adair 2015
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

add_custom_target(tests COMMENT "Build all the unit tests.")
add_custom_target(tests.quick COMMENT "Build a subset of all the unit tests to finish faster.")

##############################################################################
#   eraserface_add_unit_test(<name> ...)
#
# Equivalent to `eraserface_add_test`, except the test is also added as a
# dependency of the `tests` target.
##############################################################################

function(eraserface_add_unit_test name)
    eraserface_add_test(${ARGV})
    add_dependencies(tests ${name})
    if ((NOT "${name}" MATCHES "\\.ext\\.") AND (NOT "${name}" MATCHES "_mcd"))
        add_dependencies(tests.quick ${name})
    endif()
endfunction()

include_directories(${eraserface_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE UNIT_TESTS "*.cpp")

foreach(_file IN LISTS UNIT_TESTS)
    eraserface_target_name_for(_target "${_file}")
    add_executable(${_target} EXCLUDE_FROM_ALL "${_file}")
    eraserface_add_unit_test(${_target} ${CMAKE_CURRENT_BINARY_DIR}/${_target})
endforeach()

add_dependencies(check tests)
