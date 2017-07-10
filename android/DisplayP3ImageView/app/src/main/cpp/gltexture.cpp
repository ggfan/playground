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

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include <stb/stb_image.h>
#include "simple_png.h"
#include "color_transform.h"
#include "gltexture.h"
#include "android_assets.h"
#include "app_engine.h"


#define INVALID_TEXTURE_ID 0xFFFFFFFF
AssetTexture::AssetTexture(const std::string& name) :
  name_(name), p3Id_(INVALID_TEXTURE_ID), sRGBId_(INVALID_TEXTURE_ID),
  valid_(false), colorSpace_(DISPLAY_COLORSPACE::INVALID)
{
}

AssetTexture::~AssetTexture() {
  if (valid_) {
    glDeleteTextures(1, &p3Id_);
    glDeleteTextures(1, &sRGBId_);
    valid_ = false;
    p3Id_ = INVALID_TEXTURE_ID;
    sRGBId_ = INVALID_TEXTURE_ID;
  }
}

void AssetTexture::ColorSpace(enum DISPLAY_COLORSPACE space) {
  ASSERT(space != DISPLAY_COLORSPACE::INVALID, "invalid colorSpace_");
  // should release all of the previous valid textures...
  colorSpace_ = space;
}
DISPLAY_COLORSPACE AssetTexture::ColorSpace(void) {
  return colorSpace_;
}

bool AssetTexture::IsValid(void) { return valid_; }
GLuint AssetTexture::P3TexId() {
  ASSERT(valid_, "Texture has not created");
  return p3Id_;
}

GLuint AssetTexture::SRGBATexId() {
  ASSERT(valid_, "texture has not crated");
  return sRGBId_;
}

/*
 * CreateGLTexture()
 *     Create textures with regard to current display color space.
 *     For P3 image, one texture is created with original image; the second
 *     texture is created to from newly created image from:
 *       original image --> sRGB color Space --> display color space
 *     this way, colors outside sRGB are removed.
 */
bool AssetTexture::CreateGLTextures(AAssetManager *mgr) {
  ASSERT(mgr, "Asset Manager is not valid");
  ASSERT(colorSpace_ != DISPLAY_COLORSPACE::INVALID, "context color space not set");
  if (valid_) {
    glDeleteTextures(1, &p3Id_);
    glDeleteTextures(1, &sRGBId_);
    valid_ = false;
    p3Id_ = INVALID_TEXTURE_ID;
    sRGBId_ = INVALID_TEXTURE_ID;
  }

  glGenTextures(1, &p3Id_);
  glBindTexture(GL_TEXTURE_2D, p3Id_);

  std::vector<uint8_t> fileContent;
  AssetReadFile(mgr, name_, fileContent);
  PNGHeader header(name_, fileContent.data(), fileContent.size());

  uint32_t imgWidth, imgHeight, n;
  uint8_t* imageData = stbi_load_from_memory(
      fileContent.data(), fileContent.size(), reinterpret_cast<int*>(&imgWidth),
      reinterpret_cast<int*>(&imgHeight), reinterpret_cast<int*>(&n), 4);
  uint8_t* imgBits = imageData;
  std::vector<uint8_t> staging;
  if (colorSpace_ == DISPLAY_COLORSPACE::SRGB &&
      header.IsP3Image()) {
    staging.resize(imgWidth * imgHeight * 4 * sizeof(uint8_t));
    IMAGE_FORMAT src {
        .buf_ = imageData,
        .width_ = imgWidth,
        .height_ = imgHeight,
        .colorSpace_ = COLOR_SPACE::DISPLAY_P3,
        .format_ = COLOR_FORMAT::R8G8B8A8,
        .channels_ = 4,
        .gamma_ = header.GetGamma(),
        .npm_ = header.HasNPM() ? header.NPM() : GetTransformNPM(NPM_TYPE::P3_D65),
    };

    IMAGE_FORMAT dst {
        .buf_ = staging.data(),
        .width_ = imgWidth,
        .height_ = imgHeight,
        .colorSpace_ = COLOR_SPACE::SRGBA_UNORM,
        .format_ = COLOR_FORMAT::R8G8B8A8,
        .channels_ = 4,
        .gamma_ = src.gamma_,
        .npm_ = GetTransformNPM(NPM_TYPE::SRGB_D65_INV),
    };
    TransformColorSpace(dst, src);
    imgBits = staging.data();
  } else if(colorSpace_ == DISPLAY_COLORSPACE::P3 && !header.IsP3Image()) {
     // sRGB to P3
    staging.resize(imgWidth * imgHeight * 4 * sizeof(uint8_t));
    IMAGE_FORMAT src {
        .buf_ = imageData,
        .width_ = imgWidth,
        .height_ = imgHeight,
        .colorSpace_ = COLOR_SPACE::SRGBA_UNORM,
        .format_ = COLOR_FORMAT::R8G8B8A8,
        .channels_ = 4,
        .gamma_ = header.GetGamma(),
        .npm_ = header.HasNPM() ? header.NPM() : GetTransformNPM(NPM_TYPE::SRGB_D65)
    };

    IMAGE_FORMAT dst {
        .buf_ = staging.data(),
        .width_ = imgWidth,
        .height_ = imgHeight,
        .colorSpace_ = COLOR_SPACE::DISPLAY_P3,
        .format_ = COLOR_FORMAT::R8G8B8A8,
        .channels_ = 4,
        .gamma_ = src.gamma_,
        .npm_ = GetTransformNPM(NPM_TYPE::P3_D65_INV),
    };
    TransformColorSpace(dst, src);
    imgBits = staging.data();
  }
  glTexImage2D(GL_TEXTURE_2D, 0,  // mip level
               GL_RGBA,
               imgWidth, imgHeight,
               0,                // border color
               GL_RGBA, GL_UNSIGNED_BYTE, imgBits);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glGenTextures(1, &sRGBId_);
  glBindTexture(GL_TEXTURE_2D, sRGBId_);

  if(colorSpace_ == DISPLAY_COLORSPACE::P3 && header.IsP3Image()) {
    IMAGE_FORMAT src {
        .buf_ = imageData,
        .width_ = imgWidth,
        .height_ = imgHeight,
        .colorSpace_ = COLOR_SPACE::DISPLAY_P3,
        .format_ = COLOR_FORMAT::R8G8B8A8,
        .channels_ = 4,
        .gamma_ = header.GetGamma(),
        .npm_ = header.HasNPM() ? header.NPM() : GetTransformNPM(NPM_TYPE::P3_D65),
    };
    std::vector<uint8_t> srgbImg(imgWidth * imgHeight * 4 * sizeof(uint8_t));
    IMAGE_FORMAT dst{
        .buf_ = srgbImg.data(),
        .width_ = imgWidth,
        .height_ = imgHeight,
        .colorSpace_ = COLOR_SPACE::SRGBA_UNORM,
        .format_ = COLOR_FORMAT::R8G8B8A8,
        .channels_ = 4,
        .gamma_ = 0.0f,     // intermediate image stays in linear space
        .npm_ = GetTransformNPM(NPM_TYPE::SRGB_D65_INV),
    };
    TransformColorSpace(dst, src);

    // sRGB back to P3 so we could display it correctly on P3 device mode
    staging.resize(imgWidth * imgHeight * 4 * sizeof(uint8_t));
    IMAGE_FORMAT tmp  = src;
    src = dst;   // intermediate gamma is 0.0f
    dst = tmp;   // original src's gamma is preserved
    src.npm_ = GetTransformNPM(NPM_TYPE::SRGB_D65);
    dst.buf_ = staging.data();
    dst.npm_ = GetTransformNPM(NPM_TYPE::P3_D65_INV);
    dst.gamma_ = DEFAULT_DISPLAY_GAMMA,

    TransformColorSpace(dst, src);
    imgBits = staging.data();
  }
  glTexImage2D(GL_TEXTURE_2D, 0,  // mip level
               GL_RGBA,
               imgWidth, imgHeight,
               0,                // border color
               GL_RGBA, GL_UNSIGNED_BYTE, imgBits);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

  glBindTexture(GL_TEXTURE_2D, 0);
  stbi_image_free(imageData);
  valid_ = true;

  return true;
}

/*
 * DeleteTextures()
 *    Release all textures created in engine
 */
void DeleteTextures(struct engine* engine) {
  for (auto& tex : engine->textures_) {
    delete tex;
  }
  engine->textures_.resize(0);
}

/*
 * CreateTextures()
 *    Create 2 textures in current display color space ( P3 or sRGB)
 *    If it is P3 space, image is transformed through sRGB so colors
 *    outside sRGB gamut are removed.
 */
bool CreateTextures(struct engine * engine) {
  std::vector<std::string> files;
  AssetEnumerateFileType(engine->app->activity->assetManager, ".png", files);
  if (files.empty()) {
    return false;
  }
  DeleteTextures(engine);

  for(auto& f : files) {
    AssetTexture* tex = new AssetTexture(f);
    ASSERT(tex, "OUT OF MEMORY");
    tex->ColorSpace(engine->dispColorSpace);
    bool status = tex->CreateGLTextures(engine->app->activity->assetManager);
    ASSERT(status, "Failed to create Texture for %s", f.c_str());
    engine->textures_.push_back(tex);
  }

  return true;
}
