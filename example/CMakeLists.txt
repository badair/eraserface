# Copyright Louis Dionne 2015
# Modified Work Copyright Barrett Adair 2015
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)

add_custom_target(examples COMMENT "Build all the examples.")
add_dependencies(check examples)

include_directories(${eraserface_SOURCE_DIR}/include)

file(GLOB_RECURSE EXAMPLES "*.cpp")
file(GLOB_RECURSE EXPERIMENTAL_EXAMPLES "experimental*.cpp")

if (NOT CALLABLE_TRAITS_BUILD_EXPERIMENTAL)
    foreach(_experimental IN LISTS EXPERIMENTAL_EXAMPLES)
        list(REMOVE_ITEM EXAMPLES ${_experimental})
    endforeach()
endif()

foreach(_file IN LISTS EXAMPLES)
    eraserface_target_name_for(_target "${_file}")
    add_executable(${_target} EXCLUDE_FROM_ALL "${_file}")
    eraserface_add_test(${_target} ${CMAKE_CURRENT_BINARY_DIR}/${_target})
    add_dependencies(examples ${_target})
endforeach()
