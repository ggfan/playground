#!/bin/bash
# remove all binary libs that are downloaded
SRC_ROOT=${HOME}/pproj/openFrameworks
DST_ROOT=${HOME}/pproj/playground/openFrameworks/v_0.10.0

modules=(libs/FreeImage libs/boost libs/curl libs/freetype \
         libs/glm libs/json libs/openssl libs/pugixml \
         libs/tess2 libs/uriparser libs/utf8          \
         addons/ofxAssimpModelLoader/libs addons/ofxOpenCv/libs \
         addons/ofxPoco/libs addons/ofxSvg/libs)
for mod in ${modules[@]}; do
  rm -fr ${DST_ROOT}/${mod}
done

