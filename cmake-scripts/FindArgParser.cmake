# - Try to find cpp-arg-parser package include directories and librarires
# Usage:
# 		find_package(ArgParser)
#
# Following variables must be set before calling find_package:
#
# ArgParser_ROOT_DIR			Root installation of ArgParser package it is not found in system path
#
#
# The module defines foolowing variables:
#
# ArgParser_FOUND				ArgParser package is present in the system
# ArgParser_INCLUDE_DIR			Path to include directories
# ArgParser_LIBRARY				Path to library

find_path(ArgParser_ROOT_DIR NAMES include/cppargparser/arg_parser.hpp)

find_path(ArgParser_INCLUDE_DIR
          NAMES arg_parser.hpp
          HINTS ${ArgParser_ROOT_DIR}/include/cppargparser)

set(ArgParser_HINT_DIR ${ArgParser_ROOT_DIR}/lib)

find_library(ArgParser_LIBRARY
             NAMES argparser
             HINTS ${ArgParser_HINT_DIR})


include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_LIBRARIES ${ArgParser_LIBRARY})
set(CMAKE_REQUIRED_INCLUDES ${ArgParser_INCLUDE_DIR})
set(CMAKE_REQUIRED_FLAGS "-std=c++14")
check_cxx_source_compiles("#include <cppargparser/arg_parser.hpp>\nint main() { ArgumentParser args; return 0; }" ArgParser_LINKS_SOLO)
set(CMAKE_REQUIRED_LIBRARIES)