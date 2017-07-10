/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef  __GL_TEXTURE_H__
#define  __GL_TEXTURE_H__
#include "common.h"
#include <string>
#include <GLES3/gl32.h>
#include <android/asset_manager.h>

class AssetTexture {
private:
  std::string name_;
  GLuint p3Id_;
  GLuint sRGBId_;
  bool  valid_;
  enum DISPLAY_COLORSPACE colorSpace_;

public:
  explicit AssetTexture(const std::string& name);
  ~AssetTexture();
  void ColorSpace(enum DISPLAY_COLORSPACE  clrSpace);
  DISPLAY_COLORSPACE ColorSpace(void);
  bool CreateGLTextures(AAssetManager* mgr);
  bool IsValid(void);
  GLuint P3TexId(void);
  GLuint SRGBATexId(void);
};

bool CreateTextures(struct engine * engine);
void DeleteTextures(struct engine* engine);

#endif  // __GL_TEXTURE_H__
