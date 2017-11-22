cmake_minimum_required(VERSION 3.4.1)

get_filename_component(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../common ABSOLUTE)
add_library(native-lib SHARED
    native-lib.cpp
    ${COMMON_DIR}/wrapper/include/NeuralNetworks.c)

include_directories(${COMMON_DIR}/wrapper)
target_link_libraries(native-lib android log)
