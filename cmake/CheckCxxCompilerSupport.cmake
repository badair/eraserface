
# Copyright Louis Dionne 2015
# Copyright Markus J. Weber 2015
# Modified Work Copyright Barrett Adair 2015
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
#
#
# This CMake module checks whether the current compiler is supported, and
# provides friendly hints to the user.

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "3.5")
        message(WARNING "
    ### You appear to be using Clang ${CMAKE_CXX_COMPILER_VERSION}, 
    ### which might not be unable to compile CallableTraits. If it compiles
	### successfully, please let us know by opening an issue at 
	### https://github.com/badair/callable_traits/issues. If not, consider
	### switching to Clang >= 3.5. If it is already installed on your system,
	### you can tell CMake about it with
    ###
    ###     cmake .. -DCMAKE_CXX_COMPILER=/path/to/clang
        ")
    endif()
endif()
if (MSVC)
    if(${MSVC_VERSION} LESS 1900)
        message(WARNING "
### Your version of Visual Studio is not supported.
### Please upgrade to Visual Studio 2015 or above.
        ")
    endif()
    if(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio 14 2015*")
        message(WARNING "
### You're not using a Visual Studio 2015 Makefile generator. Please run cmake with
###     cmake .. -G\"Visual Studio 14 2015\"
        ")
    endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "6.3")
        message(WARNING "
    ### You appear to be using Apple's Clang ${CMAKE_CXX_COMPILER_VERSION}, 
    ### which is shipped with Xcode < 6.3. You should consider using a 
	### non-Apple Clang >= 3.5, which can be installed via 
	### Homebrew with
    ###
    ###     brew install llvm --with-clang
    ###
    ### You can then tell CMake to use that non-system Clang with
    ###
    ###     cmake .. -DCMAKE_CXX_COMPILER=/path/to/clang
        ")
    endif()
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    if (${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS "4.8.0")
    message(WARNING "
    ### You appear to be using GCC ${CMAKE_CXX_COMPILER_VERSION}, which might
    ### not be able to compile CallableTraits. CallableTraits officially supports
        ### GCC versions >= 4.8.0.
    ")
    endif()
else()
    message(WARNING "
    ### You appear to be using a compiler that is not yet tested with CallableTraits.
    ### Please tell us whether it successfully compiles or if and how it
    ### fails by opening an issue at https://github.com/badair/callable_traits/issues.
    ### Thanks!
    ")
endif()
