# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)
project("template")

find_package(game-activity  REQUIRED CONFIG)

add_library(${CMAKE_PROJECT_NAME} SHARED native-lib.cpp android_main.cpp)
target_link_libraries( ${CMAKE_PROJECT_NAME}
  game-activity::game-activity
  EGL  GLESv1_CM
  android log)