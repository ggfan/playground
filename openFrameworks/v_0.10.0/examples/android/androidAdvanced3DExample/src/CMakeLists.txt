cmake_minimum_required(VERSION 3.4.1)
project(OFAndroidApp LANGUAGES C CXX)


add_subdirectory(../../../../libs/openFrameworks  ${CMAKE_CURRENT_BINARY_DIR}/openFrameWorksBuild)

add_library(${PROJECT_NAME}
  SHARED
    main.cpp
    ofApp.cpp
    Grid.cpp
    OrthoCamera.cpp
    Swarm.cpp
    )

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++14 -Wall)

get_filename_component(ADDON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../addons ABSOLUTE)
get_filename_component(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../libs  ABSOLUTE)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    atomic
    android
    OpenSLES
    z
    GLESv1_CM
    GLESv2
    log
    openFrameworksLib)
