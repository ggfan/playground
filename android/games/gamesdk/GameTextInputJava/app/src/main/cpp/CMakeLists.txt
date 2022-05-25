# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

# Declares and names the project.
project("GameTextInput")

find_package(game-text-input REQUIRED CONFIG)

add_library(${PROJECT_NAME}  SHARED
        ${PROJECT_NAME}.cpp
        TextInputLib-src.cpp)

target_link_libraries(${PROJECT_NAME}
        game-text-input::game-text-input
        log)