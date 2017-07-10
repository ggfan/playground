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
#include <android/input.h>
#include "mathfu/matrix.h"
#include "mathfu/glsl_mappings.h"
#include "app_engine.h"

const uint64_t kSwipeThreshold = static_cast<uint64_t>(1000000000);
const uint32_t kMinDistance = 100;
const uint32_t kMaxTapDistance = 10;
const uint32_t kMaxTapTime   = static_cast<uint64_t>(125000000);

static mathfu::vec2 v1(0.0f, 0.0f);
static uint64_t startTime;

static void reset(void) {
  v1 = mathfu::vec2(0.0f, 0.0f);
}

/*
 * ProcessVerticalSwipeEvent()
 *
 */
static void ProcessVerticalSwipeEvent(struct engine* engine, bool up) {

  int offset = -1;
  if (up) {
    offset = 1;
    LOGE("====Release UP event");
  }

  uint32_t idx = engine->textureIdx_;
  idx += offset + engine->textures_.size();
  engine->textureIdx_ = idx % engine->textures_.size();
}

void ProcessTapEvent(struct engine* engine, int x, int y) {
  // Get current display size:  the phone is in landscape mode ( in AndroidManifest.xml )
  int halfWidth = engine->width / 2;

  if (x > halfWidth - 10  && x < halfWidth + 10)
    return;

  uint32_t  mask = 1 << (x / halfWidth);

  engine->renderModeBits_ = (engine->renderModeBits_ & mask) ?
                            (engine->renderModeBits_ & ~mask) :
                            (engine->renderModeBits_ | mask);
  UpdateUI(engine);
}

bool engine_ProcessInputEvent(struct engine* engine, const AInputEvent* event) {
  if (!engine || !event) {
    reset();
    return true;
  }

  if(AMotionEvent_getPointerCount(event) > 1) {
    reset();
    LOGE("more than one pointer action");
    return true;
  }

  int32_t action = AMotionEvent_getAction(event);
  if (action == AMOTION_EVENT_ACTION_CANCEL) {
    reset();
    return true;
  }

  if(action == AMOTION_EVENT_ACTION_DOWN) {
    v1.x = AMotionEvent_getX(event, 0);
    v1.y = AMotionEvent_getY(event, 0);
    startTime = AMotionEvent_getEventTime(event);
    return true;
  } else if(action == AMOTION_EVENT_ACTION_UP) {
    uint64_t endTime = AMotionEvent_getEventTime(event);
    if (endTime - startTime > kSwipeThreshold) {
      return true;
    }

    mathfu::vec2 v2;
    v2.x = AMotionEvent_getX(event, 0);
    v2.y = AMotionEvent_getY(event, 0);

    v2 = v2 - v1;
    if (endTime - startTime < kMaxTapTime  &&
        v2.Length() < kMaxTapDistance) {
      LOGI("Detected a tap event, (%f, %f)", v1.x, v1.y);
      ProcessTapEvent(engine, static_cast<int>(v1.x),
                      static_cast<int>(v1.y));
      return true;
    }

    if (v2.Length() < kMinDistance) {
      LOGI("---- too short to consider a swipe");
      return true;
    }

    if (std::abs(v2.x) > std::abs(v2.y)) {
      reset();
      return true;
    }

    int offset = 1;
    if (v2.y > 0) {
      // swiping down, decrease the index...
      offset = -1;
    }

    uint32_t idx = engine->textureIdx_;
    idx += offset + engine->textures_.size();
    engine->textureIdx_ = idx % engine->textures_.size();
  }

  return true;
}

void EnableWelcomeUI(struct engine* engine) {
  JNIEnv* jni;
  engine->app->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(engine->app->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "EnableUI", "(I)V");
  jni->CallVoidMethod(engine->app->activity->clazz, methodID,
                     engine->renderModeBits_);

  engine->app->activity->vm->DetachCurrentThread();
}

void EnableRenderUI(struct engine* engine) {
  JNIEnv* jni;
  engine->app->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(engine->app->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "EnableRenderUI", "()V");
  jni->CallVoidMethod(engine->app->activity->clazz, methodID);

  engine->app->activity->vm->DetachCurrentThread();
}
void UpdateUI(struct engine* engine) {
  JNIEnv* jni;
  engine->app->activity->vm->AttachCurrentThread(&jni, NULL);

  // Default class retrieval
  jclass clazz = jni->GetObjectClass(engine->app->activity->clazz);
  jmethodID methodID = jni->GetMethodID(clazz, "UpdateUI", "(I)V");
  jni->CallVoidMethod(engine->app->activity->clazz, methodID,
                     engine->renderModeBits_);

  engine->app->activity->vm->DetachCurrentThread();
  return;
}
