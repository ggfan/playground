include(ExternalProject)

cmake_minimum_required(VERSION 3.4.1)

set(PROTO_ROOT  ${CMAKE_SOURCE_DIR}/../third-party/proto-buf)
include_directories(${PROTO_ROOT}/proto-bin/include)

add_library( # Sets the name of the library.
             native-lib
             SHARED
             src/main/cpp/native-lib.cpp
             ${CMAKE_SOURCE_DIR}/../proto/box_coder.pb.cc)

find_library( # Sets the name of the path variable.
              log-lib
              log )

target_link_libraries( # Specifies the target library.
                       native-lib
                       ${log-lib}
                       ${PROTO_ROOT}/proto-bin/lib/libprotoc.a
                       ${PROTO_ROOT}/proto-bin/lib/libprotobuf.a)


#ExternalProject_Add(download_graph
#            SOURCE_DIR ${CMAKE_SOURCE_DIR}/../third-party/graphs/inception
#            URL  https://storage.googleapis.com/download.tensorflow.org/models/inception5h.zip
#            DOWNLOAD_DIR  ${CMAKE_SOURCE_DIR}/../third-party/graphs
#            DOWNLOAD_NAME inception.zip
#            CONFIGURE_COMMAND ""
#            BUILD_COMMAND ""
#            LOG_CONFIGURE 1
#            BUILD_ALWAYS 1
#            LOG_BUILD    1
#            INSTALL_COMMAND "")

ExternalProject_Add(download_proto
            SOURCE_DIR ${CMAKE_SOURCE_DIR}/../third-party/proto-buf/proto-src
            GIT_REPOSITORY https://github.com/google/protobuf.git
            DOWNLOAD_DIR  ${CMAKE_SOURCE_DIR}/../third-party/proto-buf/proto-src
#            DOWNLOAD_NAME
            UPDATE_COMMAND ""
            PATCH_COMMAND ""
            CONFIGURE_COMMAND ""
            BUILD_COMMAND ""
            LOG_CONFIGURE 0
            BUILD_ALWAYS 1
            LOG_BUILD    1
            INSTALL_COMMAND "")

add_custom_command(TARGET native-lib
            PRE_BUILD
            DEPENDS download_proto
            COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/../third-party/proto-buf/scripts/build_proto_android.sh
            COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/../third-party/proto-buf/scripts/generate_proto.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../third-party/proto-buf/scripts)

add_custom_command(OUTPUT ${CMAKE_SOURCE_DIR}/../proto/box_coder.pb.cc
            DEPENDS download_proto
            COMMAND /bin/bash ${CMAKE_SOURCE_DIR}/../third-party/proto-buf/scripts/generate_proto.sh
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/../third-party/proto-buf/scripts)

add_dependencies(native-lib download_proto)