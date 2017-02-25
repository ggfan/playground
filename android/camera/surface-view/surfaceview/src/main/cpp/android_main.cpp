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
 */

/* Description
 *     Demonstrate NDKCamera's PREVIEW mode -- hooks camera directly into
 *     display.
 *     Control:
 *         Double-Tap on Android device's screen to toggle start/stop preview
 *     Tested:
 *         Google Pixel and Nexus 6 phones
 */
#include <jni.h>
#include <cstdio>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "camera_manager.h"
#include "utils/native_debug.h"

/*
 * Very simple application stub to manage camera and display for Preview
 */
class Engine {
  public:
    explicit Engine(JNIEnv* env, jobject surface) :
            surface_(surface),
            androidCamera_(nullptr),
            cameraGranted_(true), env_(env) {}
    ~Engine() { DeleteCamera(); }

    ANativeWindow* getNativeWin(void) { return ANativeWindow_fromSurface(env_, surface_); }
    void RequestCameraPermission();
    void UpdateCameraPermission(jboolean granted);
    bool isCameraGranted(void) { return cameraGranted_; }
    void CreateCamera(void);
    void DeleteCamera(void);
    void OnDoubleTap(void);
private:
    JNIEnv* env_;
    jobject surface_;
    bool cameraGranted_;
    NativeCamera * androidCamera_;
};

void Engine::UpdateCameraPermission(jboolean granted) {
    cameraGranted_ = (granted != 0);
}

/*
 * Create a camera object for onboard BACK_FACING camera
 */
void Engine::CreateCamera(void) {
    // Camera needed to be requested at the run-time from Java SDK
    // if Not-granted, do nothing.
    if (!cameraGranted_ || !surface_) {
        LOGW("Camera Sample requires Full Camera access");
        return;
    }

    ASSERT(androidCamera_== nullptr, "CameraObject is already initialized");

    androidCamera_ = new NativeCamera(getNativeWin());

    ASSERT(androidCamera_, "Failed to Create CameraObject");
}

void Engine::DeleteCamera(void) {
    if (androidCamera_) {
        delete androidCamera_;
        androidCamera_ = nullptr;
    }
}

/*
 * Process Double Tap event from user:
 *      toggle the preview state ( start/pause )
 * The Tap event is defined by this app. Refer to ProcessAndroidInput()
 */
void Engine::OnDoubleTap(void) {
    if (androidCamera_) {
        androidCamera_->Animate();
    }
}

/*
 * The stub engine reference:
 *     need by run-time camera request callback function
 */
Engine *pEngineObj = nullptr;


//ToDO:
// 1) cache the global one env
// 2) get it into a thread to start/stop
// 3) find the right ration when creating camera session
void* CreateCamera(void *ctx) {
    reinterpret_cast<Engine*>(ctx)->CreateCamera();
    reinterpret_cast<Engine*>(ctx)->OnDoubleTap();
    return nullptr;
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_surfaceview_ViewActivity_notifyDisplaySurfaceChanged(
    JNIEnv *env, jobject instance, jobject surface) {
  delete pEngineObj;
  pEngineObj = nullptr;

  pEngineObj = new Engine(env, surface);
  ASSERT(pEngineObj, "Failed to get Engine Object");

//  pthread_t notifyThread;
//  int error = pthread_create(&notifyThread, nullptr, ::CreateCamera, pEngineObj);
//  ASSERT(error == 0, "Create Starting Camera thread failed");
  CreateCamera(pEngineObj);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_surfaceview_ViewActivity_notifyDisplaySurfaceCreated(
    JNIEnv *env, jobject instance, jobject surface) {
  pEngineObj = new Engine(env, surface);
  ASSERT(pEngineObj, "Failed to get Engine Object");

//  pthread_t notifyThread;
//  int error = pthread_create(&notifyThread, nullptr, ::CreateCamera, pEngineObj);
//  ASSERT(error == 0, "Create Starting Camera thread failed");
  CreateCamera(pEngineObj);
}

extern "C" JNIEXPORT void JNICALL
Java_com_sample_surfaceview_ViewActivity_notifyDisplaySurfaceDestroyed(
    JNIEnv *env, jobject instance, jobject surface) {
  delete pEngineObj;
  pEngineObj = nullptr;
}
