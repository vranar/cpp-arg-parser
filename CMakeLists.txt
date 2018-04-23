cmake_minimum_required(VERSION 3.0)
project(cpp_arg_parser)
include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
set(CMAKE_BUILD_TYPE Release)

include_directories(include)

set(HEADERS include/arg_parser.hpp)
set(SRCS src/arg_parser.cpp ${HEADERS})

set(UTEST_OUTPUT_DIR ${CMAKE_BINARY_DIR}/unit-tests)

add_library(cppargparser ${SRCS})
set_target_properties(cppargparser PROPERTIES OUTPUT_NAME "cppargparser")

link_directories(${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

add_executable(test-parse-option unit-tests/test-parse-option.cpp)
add_dependencies(test-parse-option cppargparser)
set_target_properties(test-parse-option PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${UTEST_OUTPUT_DIR})
target_link_libraries(test-parse-option cppargparser)

add_executable(test-mtx-options unit-tests/test-mtx-options.cpp)
add_dependencies(test-mtx-options cppargparser)
set_target_properties(test-mtx-options PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${UTEST_OUTPUT_DIR})
target_link_libraries(test-mtx-options cppargparser)

add_executable(test-option-register unit-tests/test-option-register.cpp)
add_dependencies(test-option-register cppargparser)
set_target_properties(test-option-register PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${UTEST_OUTPUT_DIR})
target_link_libraries(test-option-register cppargparser)

add_executable(test-positional-find unit-tests/test-positional-find.cpp)
add_dependencies(test-positional-find cppargparser)
set_target_properties(test-positional-find PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${UTEST_OUTPUT_DIR})
target_link_libraries(test-positional-find cppargparser)

add_executable(test-option-find unit-tests/test-option-find.cpp)
add_dependencies(test-option-find cppargparser)
set_target_properties(test-option-find PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${UTEST_OUTPUT_DIR})
target_link_libraries(test-option-find cppargparser)

enable_testing()
add_test("OptionRegistration" ${UTEST_OUTPUT_DIR}/test-option-register)
add_test("OptionFind" ${UTEST_OUTPUT_DIR}/test-option-find --useful-option)
add_test("PositionalFind" ${UTEST_OUTPUT_DIR}/test-positional-find 1)
add_test("ParseOption" ${UTEST_OUTPUT_DIR}/test-parse-option --int 1 --hex FF --string Hello --float 0.1)
add_test("MutualExclusion" ${UTEST_OUTPUT_DIR}/test-mtx-options -a)
add_test("MutualExclusion2Groups" ${UTEST_OUTPUT_DIR}/test-mtx-options -a -b)
add_test("MutualExclusionConflict" ${UTEST_OUTPUT_DIR}/test-mtx-options -a -b -c)
set_tests_properties("MutualExclusionConflict" PROPERTIES WILL_FAIL true)

install(TARGETS cppargparser
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HEADERS} DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cppargparser")
install(TARGETS cppargparser EXPORT cppargparser-targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(EXPORT cppargparser-targets DESTINATION ${CMAKE_INSTALL_LIBDIR}/cppargparser)
install(FILES cmake-scripts/cppargparser-config.cmake DESTINATION "${CMAKE_INSTALL_LIBDIR}/cppargparser")

