#!/bin/bash

if [ ! ["${OSTYPE}"=="darwin"*]  ]; then
  echo "This script only works on Mac OS, exiting"
  exit $?
fi


# Configure SDK/NDK locations so we do not depends on local.properties
export ANDROID_HOME=$HOME/dev/sdk_current
export ANDROID_NDK_HOME=$HOME/dev/android-ndk-r16b

# configurations:
#  temp file name to hold build result; it will be removed after build
#  no matter build successful or failed
BUILD_RESULT_FILE=build_result.txt

# Repo root directory
NDK_SAMPLE_REPO=/Users/gfan/proj/android-ndk

declare projects=(
    bitmap-plasma
    gles3jni
    hello-gl2
    hello-jni
    hello-libs
    hello-neon
    native-activity
    native-audio
    native-codec
    native-media
    native-plasma
    san-angeles
    teapots
    two-libs
    )

for d in "${projects[@]}"; do 
    pushd ${NDK_SAMPLE_REPO}/other-builds/ndkbuild/${d}
    ./gradlew  clean
    ./gradlew assembleDebug
    popd
done


#check for the apks that gets builds
declare apks=(
    bitmap-plasma/app/build/outputs/apk/debug/app-debug.apk
    gles3jni/app/build/outputs/apk/debug/app-debug.apk
    hello-gl2/app/build/outputs/apk/debug/app-debug.apk
    hello-jni/app/build/outputs/apk/debug/app-debug.apk
    hello-libs/app/build/outputs/apk/debug/app-debug.apk
    hello-neon/app/build/outputs/apk/arm7/debug/app-arm7-debug.apk
    native-activity/app/build/outputs/apk/debug/app-debug.apk
    native-audio/app/build/outputs/apk/debug/app-debug.apk
    native-codec/app/build/outputs/apk/debug/app-debug.apk
    native-media/app/build/outputs/apk/debug/app-debug.apk
    native-plasma/app/build/outputs/apk/debug/app-debug.apk
    san-angeles/app/build/outputs/apk/debug/app-armeabi-v7a-debug.apk
    teapots/classic-teapot/build/outputs/apk/debug/classic-teapot-debug.apk
    teapots/more-teapots/build/outputs/apk/debug/more-teapots-debug.apk
    )

rm -fr ${BUILD_RESULT_FILE}
for apk in "${apks[@]}"; do
  if [ ! -f ${NDK_SAMPLE_REPO}/other-builds/ndkbuild/${apk} ]; then
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

