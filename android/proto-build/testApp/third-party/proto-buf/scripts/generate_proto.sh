#!/bin/bash  -x
set -e

CUR_DIR=$(cd $(dirname $0); pwd -P)
pushd ${CUR_DIR}/../../../proto
${CUR_DIR}/../proto-host/bin/protoc --cpp_out=. box_coder.proto
popd
