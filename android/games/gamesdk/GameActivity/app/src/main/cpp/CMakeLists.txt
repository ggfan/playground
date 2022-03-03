
# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.18.1)

project("gameactivity")

find_package(game-activity REQUIRED CONFIG)

add_library( gameactivity SHARED
             GameActivityApp.cpp
             NativeGlueWrapper.c)

target_link_libraries( gameactivity

                       # Links the target library to the log library
                       # included in the NDK.
                       android
                       log
                       game-activity::game-activity)
