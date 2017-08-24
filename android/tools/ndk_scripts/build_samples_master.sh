#!/bin/bash

if [ ! ["${OSTYPE}"=="darwin"*]  ]; then
  echo "This script only works on Mac OS, exiting"
  exit $?
fi


# Configure SDK/NDK locations so we do not depends on local.properties
export ANDROID_HOME=$HOME/dev/sdk_current
export ANDROID_NDK_HOME=$HOME/dev/android-ndk-r16-beta1

# configurations:
#  temp file name to hold build result; it will be removed after build
#  no matter build successful or failed
BUILD_RESULT_FILE=build_result.txt

# Repo root directory
NDK_SAMPLE_REPO=/Users/gfan/tmp/android-ndk

declare projects=(audio-echo
    bitmap-plasma
    camera
    endless-tunnel
    gles3jni
    hello-gl2
    hello-jni
    hello-jniCallback
    hello-libs
    hello-neon
    native-activity
    native-audio
    native-codec
    native-media
    native-plasma
#    other-builds
    san-angeles
    sensor-graph
    teapots
    webp)

for d in "${projects[@]}"; do 
    pushd ${NDK_SAMPLE_REPO}/${d}
    ./gradlew  clean
    ./gradlew assembleDebug
    popd
done


#check for the apks that gets builds
declare apks=(
    audio-echo/app/build/outputs/apk/app-debug.apk
    bitmap-plasma/app/build/outputs/apk/app-debug.apk
    camera/basic/build/outputs/apk/basic-debug.apk
    camera/texture-view/build/outputs/apk/texture-view-debug.apk
    endless-tunnel/app/build/outputs/apk/app-debug.apk
    gles3jni/app/build/outputs/apk/app-debug.apk
    hello-gl2/app/build/outputs/apk/app-debug.apk
    hello-jni/app/build/outputs/apk/app-arm7-debug.apk
    hello-jniCallback/app/build/outputs/apk/app-debug.apk
    hello-libs/app/build/outputs/apk/app-debug.apk
    hello-neon/app/build/outputs/apk/app-arm7-debug.apk
    native-activity/app/build/outputs/apk/app-debug.apk
    native-audio/app/build/outputs/apk/app-debug.apk
    native-codec/app/build/outputs/apk/app-debug.apk
    native-media/app/build/outputs/apk/app-debug.apk
    native-plasma/app/build/outputs/apk/app-debug.apk
#    other-builds
    sensor-graph/accelerometer/build/outputs/apk/accelerometer-debug.apk
    san-angeles/app/build/outputs/apk/app-armeabi-v7a-debug.apk
    san-angeles/app/build/outputs/apk/app-arm64-v8a-debug.apk
    san-angeles/app/build/outputs/apk/app-x86-debug.apk
    teapots/classic-teapot/build/outputs/apk/classic-teapot-debug.apk
    teapots/more-teapots/build/outputs/apk/more-teapots-debug.apk
    teapots/choreographer-30fps/build/outputs/apk/choreographer-30fps-debug.apk
    webp/view/build/outputs/apk/view-arm7-debug.apk)

rm -fr ${BUILD_RESULT_FILE}
for apk in "${apks[@]}"; do
  if [ ! -f ${NDK_SAMPLE_REPO}/${apk} ]; then
    echo ${apk} does not build >> ${BUILD_RESULT_FILE}
  fi
done

if [ -f ${BUILD_RESULT_FILE} ]; then
   echo  "=======Failed Builds=======: "
   cat  ${BUILD_RESULT_FILE}
else
  echo "=======BUILD SUCCESS======="
fi

rm -fr ${BUILD_RESULT_FILE}

