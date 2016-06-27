#ifndef __WEBP_DECODE_H__
#define __WEBP_DECODE_H__
#include <android/native_window.h>
#include <android/asset_manager.h>

int decode_file(const char* in_file, ANativeWindow_Buffer* frameBuf, AAssetManager* assetMgr);
#endif // __WEBP_DECODE_H__