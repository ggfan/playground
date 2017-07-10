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

#ifndef __APP_ENGINE_H__
#define __APP_ENGINE_H__

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <string>

#include <initializer_list>
#include <memory>
#include <vector>
#include <cstdlib>
#include <cassert>

#include <android/sensor.h>
#include <android_native_app_glue.h>

#include "common.h"
#include "android_debug.h"
#include "gldebug.h"
#include "glshader.h"
#include "widecolor_ctx.h"
#include "gltexture.h"

struct engine {
  engine();

  struct android_app* app;

  ASensorManager* sensorManager;
  const ASensor* accelerometerSensor;
  ASensorEventQueue* sensorEventQueue;

  int animating;
  EGLDisplay display;
  EGLSurface surface;
  DISPLAY_COLORSPACE dispColorSpace;
  DISPLAY_FORMAT dispFormat;

  EGLContext context;
  int32_t width;
  int32_t height;

  ShaderProgram program_;

  // Input files
  std::vector<AssetTexture*> textures_;
  std::atomic<uint32_t>  textureIdx_;

  uint32_t renderModeBits_;
};

bool engine_init_display(struct engine* engine);
void engine_term_display(struct engine* engine);
int32_t engine_handle_input(struct android_app* app, AInputEvent* event);
void engine_handle_cmd(struct android_app* app, int32_t cmd);
void engine_draw_frame(struct engine* engine);
bool engine_ProcessInputEvent(struct engine* engine, const AInputEvent* event);
void EnableWelcomeUI(struct engine* engine);
void EnableRenderUI(struct engine* engine);
void UpdateUI(struct engine* engine);

#endif // __APP_ENGINE_H__
