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

#include <vector>
#include <map>
#include <GLES3/gl32.h>

#include "android_debug.h"
#include "gldebug.h"

/*
 * Create a texture and push it into 2D texture
 * Pre-Condition: 2D texture is already bound and remain bound throughout the usage
  // Working combinations:
  //     GL_RGBA/GL_RGBA/GL_UNSIGNED_BYTE
  //     GL_RGB10_A2/GL_RGBA/GL_UNSIGNED_INT_2_10_10_10_REV (border color works)
  //     GL_RGBA16F / GL_RGBA, GL_HALF_FLOAT ( border color works )
  //  The followings also work, but no filtering
  //     GL_RGB10_A2UI / GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV ( no color )
  //     GL_RGBA16UI / GL_RGBA_INTEGER, GL_UNSIGNED_SHORT ( no color )
 */

const uint32_t kImgWidth = 64;
const uint32_t kImgHeight = 64;
const uint32_t kRed = 0;
const uint32_t kGreen = 0x00;
const uint32_t kBlue = 0xFF;
void CreateTestTexRGBA8(void) {
  // GL_RGBA/GL_RGBA/GL_UNSIGNED_BYTE
  std::vector<uint32_t> bits;
  for (int r = 0; r < kImgHeight; r++) {
    for (int c = 0; c < kImgWidth; c++) {
      // assuming little endian:
      // GL_RGBA and GL_UNSIGNED_BYTE expecting in memory
      //   RED GREEN BLUE ALPHA
      // since we are in little endian, then uint32_t will put byte0 as first byte in the array
      //   byte0 -- RED
      //   byte1 -- GREEN
      //   byte2 -- BLUE
      //   byte0 -- Alpha
      bits.push_back((0xFF<<24) |
                     (kBlue<<16) |
                     (kGreen<<8)|
                     (kRed));
    }
  }

//  memset(bits.data(), 0xFF, bits.size() * 4);
  glTexImage2D(GL_TEXTURE_2D,
               0,  // mip level
               GL_RGBA,
               kImgWidth, kImgHeight,
               0,  //border color
               GL_RGBA, GL_UNSIGNED_BYTE, bits.data());
}

void CreateTestTexRGB10_A2(void) {
  //GL_RGB10_A2/GL_RGBA/GL_UNSIGNED_INT_2_10_10_10_REV
  std::vector<uint32_t> bits;
  for (int r = 0; r < kImgHeight; r++) {
    for (int c = 0; c < kImgWidth; c++) {
      // assuming little endian
      bits.push_back((0x3<<30) |
                     (kBlue<<22) |  //our RGB is defined as 8 bits, so we need shift 2 more
                     (kGreen<<12)|
                     (kRed<<2));
    }
  }
  LOGI("===== before TexImage2D");
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGB10_A2,  // Internal Format
               kImgWidth, kImgHeight, 0,
               GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV,
               bits.data());
}

// HALF_FLOAT is not supported directly -- not inside the header file
// we hardcode values into the array
void CreateTestTexRGBA16F(void) {
  std::vector<uint16_t> bits;
  for (int r = 0; r < kImgHeight; r++) {
    for (int c = 0; c < kImgWidth; c++) {
      // assuming little endian
      bits.push_back(0x7BFF);  // RED: 0 11110 111111111111 = 65504 ( max half precision )
      bits.push_back(0x7BFF);  // GREEN: 0 11110 111111111111 = 65504 ( max half precision )
      bits.push_back(0x0);  //  0  = 0
      bits.push_back(0x7BFF);  // 0 11110 111111111111 = 65504 ( max half precision )
    }
  }
  LOGI("===== before TexImage2D");
  glTexImage2D(GL_TEXTURE_2D, 0,
               GL_RGBA16F,  // Internal Format
               kImgWidth, kImgHeight, 0,
               GL_RGBA, GL_HALF_FLOAT,
               bits.data());
}

//GL_RGBA16F / GL_RGBA, GL_HALF_FLOAT ( border color works )
typedef void (*TEXTURE_CREATE_FUNC)(void);
static std::map<uint32_t, TEXTURE_CREATE_FUNC> TextureFuncs {
    {GL_RGBA, CreateTestTexRGBA8},
    {GL_RGB10_A2, CreateTestTexRGB10_A2},
    {GL_RGBA16F, CreateTestTexRGBA16F},
};
void CreateTestTex(uint32_t imageFormat) {
  for (auto& p: TextureFuncs) {
    if (p.first == imageFormat) {
      p.second();
      return;
    }
  }

  ASSERT(false, "Unknow format for texture format (%d) debug function",
         imageFormat);
}


void  PrintPixelRGBA8888(void*  buf, uint32_t size) {
  if (!buf) {
    return;
  }
  uint8_t* src = static_cast<uint8_t*>(buf);

  for(uint32_t i = 0; i < size;) {
    if(size - i >= 8) {
      LOGI("PIXEL %04x-%04x: %02x %02x %02x %02x - %02x %02x %02x %02x",
        i, (i+7), src[i], src[i+1], src[i+2], src[i+3], src[i+4], src[i+5], src[i+6], src[i+7]);
      i += 8;
    } else {
      for (; i < size; i++) {
        LOGI("PIXEL %d: %02x", i, src[i]);
      }
    }
  }
}