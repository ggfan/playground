#!/bin/bash

REPO_ON_GITHUB=git@github.com:googlesamples/android-ndk.git


# Configure SDK/NDK locations so we do not depends on local.properties
export ANDROID_HOME=$HOME/dev/sdk
export ANDROID_NDK_HOME=$HOME/dev/ndk/android-ndk-r18-beta1

# configurations:
#  temp file name to hold build result; it will be removed after build
#  no matter build successful or failed
BUILD_RESULT_FILE=build_result.txt

# Repo root directory
NDK_SAMPLE_REPO=${HOME}/tmp/android-ndk

declare cmakeProjs=(
    audio-echo
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
    san-angeles
    sensor-graph
    teapots)
#    webp)
declare ndkbuildProjs=(
    bitmap-plasma hello-gl2       hello-jni    hello-libs
    hello-neon    native-activity native-audio native-codec
    native-media  native-plasma   san-angeles  teapots
    )

#check for the cmakeApks that gets builds
declare cmakeApks=(
    audio-echo/app/build/outputs/apk/debug/app-debug.apk
    bitmap-plasma/app/build/outputs/apk/debug/app-debug.apk
    camera/basic/build/outputs/apk/debug/basic-debug.apk
    camera/texture-view/build/outputs/apk/debug/texture-view-debug.apk
    endless-tunnel/app/build/outputs/apk/debug/app-debug.apk
    gles3jni/app/build/outputs/apk/debug/app-debug.apk
    hello-gl2/app/build/outputs/apk/debug/app-debug.apk
    hello-jni/app/build/outputs/apk/arm7/debug/app-arm7-debug.apk
    hello-jniCallback/app/build/outputs/apk/debug/app-debug.apk
    hello-libs/app/build/outputs/apk/debug/app-debug.apk
    hello-neon/app/build/outputs/apk/debug/app-debug.apk
    native-activity/app/build/outputs/apk/debug/app-debug.apk
    native-audio/app/build/outputs/apk/debug/app-debug.apk
    native-codec/app/build/outputs/apk/debug/app-debug.apk
    native-media/app/build/outputs/apk/debug/app-debug.apk
    native-plasma/app/build/outputs/apk/debug/app-debug.apk
    sensor-graph/accelerometer/build/outputs/apk/debug/accelerometer-debug.apk
    san-angeles/app/build/outputs/apk/debug/app-armeabi-v7a-debug.apk
    san-angeles/app/build/outputs/apk/debug/app-arm64-v8a-debug.apk
    san-angeles/app/build/outputs/apk/debug/app-x86-debug.apk
    teapots/classic-teapot/build/outputs/apk/debug/classic-teapot-debug.apk
    teapots/more-teapots/build/outputs/apk/debug/more-teapots-debug.apk
    teapots/choreographer-30fps/build/outputs/apk/debug/choreographer-30fps-debug.apk
)

declare ndkbuildApks=(
    bitmap-plasma/app/build/outputs/apk/debug/app-debug.apk
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

# clone github sample repo
if [ -e ${NDK_SAMPLE_REPO} ] ; then
   rm -fr ${NDK_SAMPLE_REPO}
fi
git clone ${REPO_ON_GITHUB} ${NDK_SAMPLE_REPO}

if [ ! -e ${ANDROID_HOME} ] ; then
    echo Error: ANDROID_HOME not set -- exit
    exit -1
fi
if [ ! -e ${ANDROID_NDK_HOME} ] ; then
    echo Error: ANDROID_NDK_HOME not set -- exit
    exit -1
fi


# build cmake's things
rm -fr ${BUILD_RESULT_FILE}
for d in "${cmakeProjs[@]}"; do 
    pushd ${NDK_SAMPLE_REPO}/${d}
    ./gradlew  clean
    ./gradlew assembleDebug
    popd
done

for apk in "${cmakeApks[@]}"; do
  if [ ! -f ${NDK_SAMPLE_REPO}/${apk} ]; then
    echo cmake ${apk} does not build >> ${BUILD_RESULT_FILE}
  fi
done

# build ndkbuild's things
rm -fr ${BUILD_RESULT_FILE}
for d in "${ndkbuildProjs[@]}"; do 
    pushd ${NDK_SAMPLE_REPO}/other-builds/ndkbuild/${d}
    ./gradlew  clean
    ./gradlew assembleDebug
    popd
done

for apk in "${ndkbuildApks[@]}"; do
  if [ ! -f ${NDK_SAMPLE_REPO}/other-builds/ndkbuild/${apk} ]; then
    echo ndkbuiild ${apk} does not build >> ${BUILD_RESULT_FILE}
  fi
done


if [ -f ${BUILD_RESULT_FILE} ]; then
   echo  "=======Failed Builds=======: "
   cat  ${BUILD_RESULT_FILE}
else
  echo "=======BUILD SUCCESS======="
fi

rm -fr ${BUILD_RESULT_FILE}

