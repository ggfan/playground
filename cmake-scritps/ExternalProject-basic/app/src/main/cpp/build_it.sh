#!/bin/bash
export PATH=/opt/local/bin:/opt/local/sbin:${PATH}
export NDK_ROOT=/Users/gfan/dev/android-ndk-r12b
cd /Users/gfan/pproj/tensorflow.org && tensorflow/contrib/makefile/build_all_android.sh



