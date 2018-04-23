get_filename_component(cppargparser_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${cppargparser_DIR}/cppargparser-targets.cmake)
get_filename_component(cppargparser_INCLUDE_DIRS "${SELF_DIR}/../../include/cppargparser" ABSOLUTE)
