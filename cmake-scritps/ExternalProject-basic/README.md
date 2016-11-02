ExternalProject_Add Basic
========================
Demos steps:
- downloads
- build with shellscript

```javascript
ExternalProject_Add(download_graph
            SOURCE_DIR $ENV{HOME}/graphs/inception
            URL  https://storage.googleapis.com/download.tensorflow.org/models/inception5h.zip
            DOWNLOAD_DIR  $ENV{HOME}/graphs
            DOWNLOAD_NAME inception.zip
            CONFIGURE_COMMAND ""
            BUILD_COMMAND bash -c -k ${CMAKE_SOURC_DIR}/build_it.sh
            LOG_CONFIGURE 1
            BUILD_ALWAYS 1
            LOG_BUILD    1
            BUILD_BYPRODUCTS $ENV{HOME}/graphs/inception/tensorflow_inception_graph.pb
            INSTALL_COMMAND "")

add_dependencies(hello-jni  download_graph)
```
Because ExternalProject_Add() put commands in its own execute_process(...), there really no relationship between commands, so setting up the environment variable for executing shell is not possible directly. My workaround is:    
put everything including environment setting into a shell script and call it, the build_it.sh:
```javascript
#!/bin/bash
export PATH=/opt/local/bin:/opt/local/sbin:${PATH}
export NDK_ROOT=/Users/gfan/dev/android-ndk-r12b
cd /Users/gfan/pproj/tensorflow.org && tensorflow/contrib/makefile/build_all_android.sh
```
Noticed Potential Problems
-------------------------
in the command environment, it probably only have the basic setting, not the login shell
so should set up all of the variables manually. ${ANDROID_NDK_HOME} is not accessible
-- it has to be hardcoded...

Todo
----
Use configure file to set environment variables


The Good
--------
It works with test on Mac. To extended to windows, just need to do someting like
 set(build_command cmd.exe -k ${CMAKE_SOURCE_DIR}/build_it.bat)
 BUILD_COMMAND buidl_command
 ...





