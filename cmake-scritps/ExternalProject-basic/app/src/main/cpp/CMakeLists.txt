include(ExternalProject)

cmake_minimum_required(VERSION 3.4.1)

add_library(hello-jni SHARED
            hello-jni.c)

# Include libraries needed for hello-jni lib
target_link_libraries(hello-jni
                      android
                      log)

ExternalProject_Add(download_graph
            SOURCE_DIR $ENV{HOME}/graphs/inception
            URL  https://storage.googleapis.com/download.tensorflow.org/models/inception5h.zip
            DOWNLOAD_DIR  $ENV{HOME}/graphs
            DOWNLOAD_NAME inception.zip
            CONFIGURE_COMMAND ""
            BUILD_COMMAND bash -c -k ${CMAKE_SOURCE_DIR}/build_it.sh
            LOG_CONFIGURE 1
            BUILD_ALWAYS 1
            LOG_BUILD    1
            BUILD_BYPRODUCTS $ENV{HOME}/graphs/inception/tensorflow_inception_graph.pb
            INSTALL_COMMAND "")

add_dependencies(hello-jni  download_graph)
