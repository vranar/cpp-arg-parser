cmake_minimum_required(VERSION 3.0)
project(cpp_arg_parser)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_library(argparser_shared SHARED src/arg_parser.cpp)
add_library(argparser_static STATIC src/arg_parser.cpp)

#add_executable(test-option-find unit-tests/test-option-find.cpp)
#set_target_properties(test-option-find PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/unit-tests)

#add_executable(test-parse-option unit-tests/test-parse-option.cpp)
#set_target_properties(test-parse-option PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/unit-tests)


#        unit-tests/test-option-register.cpp
#        unit-tests/test-parse-option.cpp
#        unit-tests/test-positional-find.cpp)
