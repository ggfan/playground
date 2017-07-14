cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

add_library(native-audio-jni SHARED
            PlaySine.cpp)

# Include libraries needed for native-audio-jni lib
target_link_libraries(native-audio-jni
                      android
                      log
                      OpenSLES)
