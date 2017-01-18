#!/bin/bash -x
# Build proto from this_host for ANDROID
# THIS SCRIPTS should be started inside hosting directory of
# THIS file.
# Assumptions - the following are pre-installed on your host:
#    autoconf automake libtool curl make g++ unzip
# Refer to https://github.com/google/protobuf/blob/master/src/README.md

set -e

ANDROID_NDK_HOME=$HOME/dev/android-ndk-r12b

#Top level of protobuf directory
CUR_DIR=$(cd $(dirname $0); pwd -P)
PROTO_ROOT=$(cd ${CUR_DIR}/..; pwd -P)
PROTO_BUILD="${PROTO_ROOT}/proto-bin"

if [ -d ${PROTO_BUILD}/lib ] && [ -f ${PROTO_BUILD}/lib/libprotobuf.a ]
    then
        echo  "proto already existing for android, exiting"
        exit 0
fi

PROTO_SRC="${PROTO_ROOT}/proto-src"
if [ ! -d "${PROTO_SRC}" ]
	then
	   pushd ${CUR_DIR}/..
	   git clone https://github.com/google/protobuf.git ${PROTO_SRC}
	   popd
fi
PROTO_HOST=${PROTO_ROOT}/proto-host
if [ ! -f ${PROTO_HOST}/bin/protoc ]
      then
            echo "====Building for host proto"
            mkdir -p ${PROTO_HOST}
            pushd ${PROTO_SRC}
            pwd
            ./autogen.sh
            ./configure --disable-shared --prefix=${PROTO_HOST}
            make -j8
            make install
            popd
            echo "====Done building for host proto"
fi


pushd ${PROTO_SRC}

# Configure the target platform to build for...
TOOLCHAIN_TYPE=arm-linux-androideabi-4.9
case ${OSTYPE} in
  darwin*)
    HOST_OS=darwin
    HOST_CPU_COUNT=$(sysctl hw.ncpu | awk '{print $2}')
    ;;
  linux-gnu*)
    HOST_OS=linux
    HOST_CPU_COUNT=$(grep processor /proc/cpuinfo | wc -l)
    ;;
  cygwin*)
    HOST_OS=cygwin
    HOST_CPU_COUNT=$(grep processor /proc/cpuinfo | wc -l)
    ;;
  *)
   echo "====error: Unsupported OS: ${OSTYPE}"; exit 1;;
esac

HOST_ARCH=x86_64
ANDROID_API_LEVEL="android-21"
ANDROID_ARCH=arm
ANDROID_ABI_TYPE=armeabi-v7a

HOST_TYPE="${ANDROID_ARCH}-linux-androideabi"
export PATH=\
"${ANDROID_NDK_HOME}/toolchains/${TOOLCHAIN_TYPE}/prebuilt/${HOST_OS}-${HOST_ARCH}/bin:$PATH"
export SYSROOT=\
"${ANDROID_NDK_HOME}/platforms/${ANDROID_API_LEVEL}/arch-${ANDROID_ARCH}"
export CC="${cc_prefix} ${HOST_TYPE}-gcc --sysroot=${SYSROOT}"
export CXX="${cc_prefix} ${HOST_TYPE}-g++ --sysroot=${SYSROOT}"
export LD="${cc_prefix} ${HOST_TYPE}-ld --sysroot=${SYSROOT}"
export LDFLAGS="-L${SYSROOT}/usr/lib"

# we are using gnustl_static/gnustl_shared
export CXXSTL=\
"${ANDROID_NDK_HOME}/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ANDROID_ABI_TYPE}"

./autogen.sh

./configure  --verbose --host="${HOST_TYPE}" --enable-cross-compile --disable-shared \
             --with-sysroot="${SYSROOT}" --with-protoc="${PROTO_HOST}/bin/protoc" \
             --prefix=${PROTO_BUILD} \
             CFLAGS="-march=armv7-a" \
             CXXFLAGS="-frtti -fexceptions -march=armv7-a \
             -I${ANDROID_NDK_HOME}/sources/android/support/include \
             -I${ANDROID_NDK_HOME}/sources/cxx-stl/gnu-libstdc++/4.9/include \
             -I${ANDROID_NDK_HOME}/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ANDROID_ABI_TYPE}/include" \
             LDFLAGS="-L${ANDROID_NDK_HOME}/sources/cxx-stl/gnu-libstdc++/4.9/libs/${ANDROID_ABI_TYPE}" \
             LIBS="-lz -lgnustl_static -llog"

make  -j ${HOST_CPU_COUNT}
make install
popd

