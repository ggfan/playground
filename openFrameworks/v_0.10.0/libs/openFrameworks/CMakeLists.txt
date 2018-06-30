#Build openFrameworksLib shared lib

cmake_minimum_required(VERSION 3.4.1)

get_filename_component(ADDON_DIR
   ${CMAKE_CURRENT_SOURCE_DIR}/../../addons ABSOLUTE)

add_library(openFrameworksLib
  STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/3d/of3dPrimitives.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3d/of3dUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3d/ofCamera.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3d/ofEasyCam.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/3d/ofNode.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/app/ofAppRunner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app/ofMainLoop.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app/ofAppNoWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/app/ofBaseApp.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/communication/ofArduino.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/communication/ofSerial.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/events/ofEvents.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofBufferObject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofGLProgrammableRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofGLUtils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofMaterial.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofTexture.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofVboMesh.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofFbo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofGLRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofLight.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofShader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gl/ofVbo.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/of3dGraphics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofGraphicsBaseTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofPath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofTessellator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofBitmapFont.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofGraphics.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofPixels.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofTrueTypeFont.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofImage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics/ofRendererCollection.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/math/ofMath.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/ofMatrix3x3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/ofMatrix4x4.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/ofQuaternion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/ofVec2f.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/ofVec4f.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/sound/ofSoundBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sound/ofSoundStream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sound/ofSoundBaseTypes.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/sound/ofSoundPlayer.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofFileUtils.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofLog.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofSystemUtils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofTimer.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofUtils.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofFpsCounter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofMatrixStack.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofThread.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofURLFileLoader.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utils/ofXml.cpp

	${CMAKE_CURRENT_SOURCE_DIR}/types/ofBaseTypes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/types/ofColor.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/types/ofParameter.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/types/ofParameterGroup.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/types/ofRectangle.cpp


    ${CMAKE_CURRENT_SOURCE_DIR}/video/ofVideoPlayer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/video/ofVideoGrabber.cpp


    ${ADDON_DIR}/ofxAndroid/src/ofAppAndroidWindow.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidAccelerometer.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidLogChannel.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidSoundPlayer.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidSoundStream.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidUtils.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidVibrator.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidVideoGrabber.cpp
    ${ADDON_DIR}/ofxAndroid/src/ofxAndroidVideoPlayer.cpp

    ${ADDON_DIR}/ofxAccelerometer/src/ofxAccelerometer.cpp

)
target_compile_options(openFrameworksLib
  PRIVATE
    -std=c++14 -Wall
    -Wno-deprecated-register -Wno-unused-private-field
    -Wno-unused-lambda-capture -Wno-format-security)

get_filename_component(ADDON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../addons ABSOLUTE)
get_filename_component(LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../libs  ABSOLUTE)

target_include_directories(openFrameworksLib
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/types
    ${CMAKE_CURRENT_SOURCE_DIR}/3d
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/communication
    ${CMAKE_CURRENT_SOURCE_DIR}/events
    ${CMAKE_CURRENT_SOURCE_DIR}/gl
    ${CMAKE_CURRENT_SOURCE_DIR}/graphics
    ${CMAKE_CURRENT_SOURCE_DIR}/math
    ${CMAKE_CURRENT_SOURCE_DIR}/sound
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/video

    ${ADDON_DIR}/ofxAndroid/src
    ${ADDON_DIR}/ofxAccelerometer/src

    ${LIBS_DIR}/FreeImage/include
    ${LIBS_DIR}/boost/include
    ${LIBS_DIR}/boost/include/boost
    ${LIBS_DIR}/cairo/include
    ${LIBS_DIR}/cairo/include/cairo
    ${LIBS_DIR}/freetype/include
    ${LIBS_DIR}/freetype/include/freetype2
    ${LIBS_DIR}/freetype/include/freetype2/config
    ${LIBS_DIR}/freetype/include/freetype2/internal
    ${LIBS_DIR}/freetype/include/freetype2/internal/services
    ${LIBS_DIR}/glfw/include
    ${LIBS_DIR}/glfw/include/GLFW
    ${LIBS_DIR}/openssl/include
    ${LIBS_DIR}/openssl/include/openssl
    ${LIBS_DIR}/poco/include
    ${LIBS_DIR}/tess2/include
    ${LIBS_DIR}/utf8cpp/include
    ${LIBS_DIR}/json/include
    ${LIBS_DIR}/kiss/include
    ${LIBS_DIR}/glm/include
    ${LIBS_DIR}/utf8/include
    ${LIBS_DIR}/uriparser/include
    ${LIBS_DIR}/curl/include
    ${LIBS_DIR}/pugixml/include
    ${LIBS_DIR}/libxml2/include
    ${LIBS_DIR}/libs/glu/include_android

    ${ADDON_DIR}/ofxAndroid/src
    ${ADDON_DIR}/ofxAccelerometer/src
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wno-format-security")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

# import and set 3rd party libs
set(Third_Party_Lib_Names  freeimage boost_filesystem boost_system freetype tess2 curl ssl     crypto pugixml)
set(Third_Party_Lib_Paths  FreeImage boost            boost        freetype tess2 curl openssl openssl pugixml)
list(LENGTH Third_Party_Lib_Names Lib_Len)
math(EXPR Lib_Len "${Lib_Len} - 1")
foreach(idx RANGE 0 ${Lib_Len} 1)
    list(GET Third_Party_Lib_Names ${idx} LIB_NAME)
    list(GET Third_Party_Lib_Paths ${idx} LIB_PATH)
    
    add_library(${LIB_NAME} STATIC IMPORTED)
    set_target_properties(${LIB_NAME} PROPERTIES IMPORTED_LOCATION
       ${LIBS_DIR}/${LIB_PATH}/lib/android/${ANDROID_ABI}/lib${LIB_NAME}.a)
    target_link_libraries(openFrameworksLib PRIVATE ${LIB_NAME})
endforeach()


add_library(uriParser STATIC IMPORTED)
set_target_properties(uriParser
  PROPERTIES IMPORTED_LOCATION
    ${LIBS_DIR}/uriparser/lib/android/${ANDROID_ABI})
target_link_libraries(openFrameworksLib PRIVATE uriParser)

target_link_libraries(openFrameworksLib
  PRIVATE
	atomic android OpenSLES z
	GLESv1_CM GLESv2 log)


# What we missed:
#  ADDON_CFLAGS []
#  ADDON_LIBS []
#  ADDON_SOURCES []