cmake_minimum_required(VERSION 3.4.1)
project(OFAndroidApp LANGUAGES C CXX)

add_library(${PROJECT_NAME}
  SHARED
    main.cpp
    ofApp.cpp)

target_compile_options(${PROJECT_NAME}
  PRIVATE
    -Wall)

# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} '-Wl,--as-needed' \
#                                                            '-Wl,--gc-section'")
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    android
    log
    atomic
    OpenSLES
    z
    GLESv1_CM
    GLESv2)