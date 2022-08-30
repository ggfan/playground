/*
 * Copyright (C) 2010 The Android Open Source Project
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

//BEGIN_INCLUDE(all)
#include <initializer_list>
#include <memory>
#include <cstdlib>
#include <cstring>
#include <jni.h>
#include <cerrno>
#include <cassert>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include <android/sensor.h>
#include <android/log.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "game-activity-template", __VA_ARGS__))
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "game-activity-template", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "game-activity-template", __VA_ARGS__))

/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    float_t x;
    float_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
    bool softInputVisible;
};

extern "C" void android_main(struct android_app* app);

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config = nullptr;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, nullptr, nullptr);

    /* Here, the application chooses the configuration it desires.
     * find the best match if possible, otherwise use the very first one
     */
    eglChooseConfig(display, attribs, nullptr,0, &numConfigs);
    std::unique_ptr<EGLConfig[]> supportedConfigs(new EGLConfig[numConfigs]);
    assert(supportedConfigs);
    eglChooseConfig(display, attribs, supportedConfigs.get(), numConfigs, &numConfigs);
    assert(numConfigs);
    auto i = 0;
    for (; i < numConfigs; i++) {
        auto& cfg = supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            r == 8 && g == 8 && b == 8 && d == 0 ) {

            config = supportedConfigs[i];
            break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }

    if (config == nullptr) {
        LOGW("Unable to initialize EGLConfig");
        return -1;
    }

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, engine->app->window, nullptr);
    context = eglCreateContext(display, config, nullptr, nullptr);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;

    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        LOGI("OpenGL Info: %s", info);
    }
    // Initialize GL state.
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glDisable(GL_DEPTH_TEST);

    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == nullptr) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
    glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
                 ((float)engine->state.y)/engine->height, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */

static int32_t engine_handle_input(struct android_app* app) {
    auto* engine = (struct engine*)app->userData;
    auto ib = android_app_swap_input_buffers(app);
    if (ib && ib->motionEventsCount) {
        LOGI("MotionEvent ib (%p) Got %d MotionEvents", ib, (int32_t)ib->motionEventsCount);
        for (int i = 0; i < ib->motionEventsCount; i++) {
            auto *event = &ib->motionEvents[i];
            int32_t ptrIdx = 0;
            switch (event->action & AMOTION_EVENT_ACTION_MASK) {
                case AMOTION_EVENT_ACTION_POINTER_DOWN:
                case AMOTION_EVENT_ACTION_POINTER_UP:
                    // Retrieve the index for the starting and the ending of any secondary pointers
                    ptrIdx = (event->action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
                                                                                       AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
                case AMOTION_EVENT_ACTION_DOWN:
                case AMOTION_EVENT_ACTION_UP:
                    engine->state.x = GameActivityPointerAxes_getAxisValue(&event->pointers[ptrIdx],
                                                                           AMOTION_EVENT_AXIS_X);
                    engine->state.y = GameActivityPointerAxes_getAxisValue(&event->pointers[ptrIdx],
                                                                           AMOTION_EVENT_AXIS_Y);
                    LOGI("ptrIdx: %d, coordinates buffer:(%f, %f), (%f, %f)",
                         ptrIdx,
                         GameActivityPointerAxes_getAxisValue(&event->pointers[0],
                                                              AMOTION_EVENT_AXIS_X),
                         GameActivityPointerAxes_getAxisValue(&event->pointers[0],
                                                              AMOTION_EVENT_AXIS_Y),
                         GameActivityPointerAxes_getAxisValue(&event->pointers[1],
                                                              AMOTION_EVENT_AXIS_X),
                         GameActivityPointerAxes_getAxisValue(&event->pointers[1],
                                                              AMOTION_EVENT_AXIS_Y));
                    break;
                case AMOTION_EVENT_ACTION_MOVE:
                    // Process the move action: the new coordinates for all active touch pointers
                    // are inside the event->pointers[]. Compare with our internally saved
                    // coordinates to find out which pointers are actually moved. Note that there is
                    // no index embedded inside event->action for AMOTION_EVENT_ACTION_MOVE (there
                    // might be multiple pointers moved at the same time).
                    // ...
                    break;
                default:
                    break;
            }

#if 0
            if (action  == AMOTION_EVENT_ACTION_DOWN  ||
                action  == AMOTION_EVENT_ACTION_POINTER_DOWN) {
                // toggle the soft input
                if (engine->softInputVisible) {
                    GameActivity_hideSoftInput(app->activity, 0);
                } else {
                    GameActivity_showSoftInput(app->activity, 0);
                }
                engine->softInputVisible = !engine->softInputVisible;
            }
#endif
        }
        android_app_clear_motion_events(ib);
    }

    // process the KeyEvent in a similar way.

    return 0;
}

static void print_current_orientation(android_app* app) {
#if 0
    // Option1: use AAssetManager_fromJava(env, jobject assetManager)
    // described from asset_manager_jni.h

    //Option2: get from cached assetManager native object.
    auto cfg = AConfiguration_new();
    AConfiguration_fromAssetManager(cfg, app->activity->assetManager);
    auto orientation_from_asset = AConfiguration_getOrientation(cfg);
    LOGI("=====TestOrientation: APP_CMD_CONFIG_CHANGED::Native orientation: %d",
         orientation_from_asset);
    AConfiguration_delete(cfg);

    // Option3: use cached config object
    auto orientation = AConfiguration_getOrientation(app->config);
    LOGI("=====GA-Orientation: APP_CMD_CONFIG_CHANGED handler(%d)", orientation);

    // Summary/Conclusion: none of them gives the right orientation when it gets changed.
    // this is a bug in GameActivity/NativeActivity, not sure what the fix could be.
#endif
}
/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
    auto* engine = (struct engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != nullptr) {
                engine_init_display(engine);
                engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                               engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                               engine->accelerometerSensor,
                                               (1000L/60)*1000);
            }
#if 0
            GameActivity_showSoftInput(app->activity,
                                       GameActivityShowSoftInputFlags::GAMEACTIVITY_SHOW_SOFT_INPUT_FORCED);
            engine->softInputVisible = true;
#endif
            engine->animating = 1;
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            GameActivity_hideSoftInput(app->activity, GAMEACTIVITY_HIDE_SOFT_INPUT_NOT_ALWAYS);
            engine->softInputVisible = false;
            engine_draw_frame(engine);
            break;
        case APP_CMD_CONFIG_CHANGED:
            print_current_orientation(app);
            break;
        default:
            break;
    }
}

/*
 * AcquireASensorManagerInstance(void)
 *    Workaround ASensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
#include <dlfcn.h>
ASensorManager* AcquireASensorManagerInstance(android_app* app) {

    if(!app)
        return nullptr;

    typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
    void* androidHandle = dlopen("libandroid.so", RTLD_NOW);
    auto getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
            dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
    if (getInstanceForPackageFunc) {
        JNIEnv* env = nullptr;
        app->activity->vm->AttachCurrentThread(&env, nullptr);

        jclass android_content_Context = env->GetObjectClass(app->activity->javaGameActivity);
        jmethodID midGetPackageName = env->GetMethodID(android_content_Context,
                                                       "getPackageName",
                                                       "()Ljava/lang/String;");
        auto packageName= (jstring)env->CallObjectMethod(app->activity->javaGameActivity,
                                                         midGetPackageName);

        const char *nativePackageName = env->GetStringUTFChars(packageName, nullptr);
        ASensorManager* mgr = getInstanceForPackageFunc(nativePackageName);
        env->ReleaseStringUTFChars(packageName, nativePackageName);
        app->activity->vm->DetachCurrentThread();
        if (mgr) {
            dlclose(androidHandle);
            return mgr;
        }
    }

    typedef ASensorManager *(*PF_GETINSTANCE)();
    auto getInstanceFunc = (PF_GETINSTANCE)
            dlsym(androidHandle, "ASensorManager_getInstance");
    // by all means at this point, ASensorManager_getInstance should be available
    assert(getInstanceFunc);
    dlclose(androidHandle);

    return getInstanceFunc();
}

extern "C" void GameTextInputGetStateCB(void *ctx, const struct GameTextInputState *state) {
    auto* engine = (struct engine*)ctx;
    if (!engine || !state) return;
    LOGI("InputText:: buffer = %s, selection(%d, %d), compose(%d, %d), string:%s",
         state->text_UTF8, state->selection.start, state->selection.end,
         state->composingRegion.start, state->composingRegion.end,
         &(state->text_UTF8[state->selection.start]));
    engine->app->textInputState = 0;
}

void testContentRect(android_app* app);
void dumpEventMemFootPrint(void);
/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
    struct engine engine{};

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    engine.app = state;

    android_app_set_key_event_filter(state, NULL);
    android_app_set_motion_event_filter(state, NULL);

    // Prepare to monitor accelerometer
    engine.sensorManager = AcquireASensorManagerInstance(state);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(
            engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(
            engine.sensorManager,
            state->looper, LOOPER_ID_USER,
            nullptr, nullptr);

    if (state->savedState != nullptr) {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }

    // loop waiting for stuff to do.
    dumpEventMemFootPrint();
    while (true) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, nullptr, &events,
                                      (void**)&source)) >= 0) {

            // Process this event.
            if (source != nullptr) {
                source->process(source->app, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                if (engine.accelerometerSensor != nullptr) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                                                       &event, 1) > 0) {
#if (0)
                        LOGI("accelerometer: x=%f y=%f z=%f",
                             event.acceleration.x, event.acceleration.y,
                             event.acceleration.z);
#endif
                    }
                }
            }

            // Check if we are exiting.
            if (state->destroyRequested != 0) {
                engine_term_display(&engine);
                return;
            }
        }
        engine_handle_input(state);

        if (state->textInputState) {
            GameActivity_getTextInputState(state->activity, GameTextInputGetStateCB, &engine);
        }
        if (engine.animating) {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }

            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine_draw_frame(&engine);

            testContentRect(state);
        }
    }
}


void testContentRect(android_app* app) {
    LOGI("\n====>Rect(%d,%d; %d, %d), pendingRect(%d, %d - %d, %d)",
         app->contentRect.left, app->contentRect.top,
         app->contentRect.right, app->contentRect.bottom,
         app->pendingContentRect.left, app->pendingContentRect.top,
         app->pendingContentRect.right, app->pendingContentRect.bottom);
}

// helper functions
void dumpGameActivityMotionEvent (GameActivityMotionEvent* e) {
    if (!e) return;
    LOGD(" \n====>: id(%d), source(%d), action(%d), flags(%#x), metaState(%#x)"
         "\n-----: actionButton(%d), buttonState(%x), classification(%d), edgeFlags(%d)"
         "\n-----: pointerCount(%d)",
         e->deviceId,e->source,e->action,e->flags,e->metaState,
         e->actionButton, e->buttonState, e->classification, e->edgeFlags,
         e->pointerCount);
    /* int64_t eventTime;
       int64_t downTime; */

    // LOGD("-----: pointerCount(%d)", e->pointerCount);
    for (int idx = 0; idx < e->pointerCount; idx++) {
        LOGD("ptrIdx(%d): (%f, %f)", idx,
             GameActivityPointerAxes_getAxisValue(&e->pointers[idx],
                                                  AMOTION_EVENT_AXIS_X),
             GameActivityPointerAxes_getAxisValue(&e->pointers[idx],
                                                  AMOTION_EVENT_AXIS_Y));
    }
    LOGD("<====");

    /*
    GameActivityPointerAxes
            pointers[GAMEACTIVITY_MAX_NUM_POINTERS_IN_MOTION_EVENT];

    float precisionX;
    float precisionY;
     */
}

#define NAME_AND_SIZE(x) #x, sizeof(x)
#define NAME_AND_VALUE(x) #x, x
void dumpEventMemFootPrint(void) {
    auto totalSize = sizeof (android_input_buffer);
    auto motionEventSize = sizeof(GameActivityMotionEvent);
    auto motionAxisSize  = sizeof (GameActivityPointerAxes);
    auto keyEventSize = sizeof(GameActivityKeyEvent);

    LOGD("\n====>%s(%d), %s(%d), %s(%d), %s(%d)"
         "\n-----%s(%d), %s(%d), %s(%d), %s(%d)",
         NAME_AND_SIZE(android_input_buffer),
         NAME_AND_SIZE(GameActivityMotionEvent),
         NAME_AND_SIZE(GameActivityPointerAxes),
         NAME_AND_SIZE(GameActivityKeyEvent),
         NAME_AND_VALUE(NATIVE_APP_GLUE_MAX_NUM_MOTION_EVENTS),
         NAME_AND_VALUE(GAMEACTIVITY_MAX_NUM_POINTERS_IN_MOTION_EVENT),
         NAME_AND_VALUE(GAME_ACTIVITY_POINTER_INFO_AXIS_COUNT),
         NAME_AND_VALUE(NATIVE_APP_GLUE_MAX_NUM_KEY_EVENTS)

    // #android_input_buffer, sizeof(android_input_buffer),
    //#GameActivityMotionEvent, sizeof(GameActivityMotionEvent),
    // #GameActivityPointerAxes, sizeof(GameActivityPointerAxes),
    // #GameActivityKeyEvent,  sizeof(GameActivityKeyEvent)
    );
}
#undef NAME_AND_VALUE
#undef NAME_AND_SIZE

//END_INCLUDE(all)

#include <android/asset_manager_jni.h>
#include <jni.h>

jobject globalJavaAssetMgr = nullptr;
AAssetManager* globalAssetMgr = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_native_1activity_MainActivity_testOrientationNative(JNIEnv *env, jobject thiz,
        jobject asset_mgr) {
    if (!globalJavaAssetMgr) {
        globalJavaAssetMgr = env->NewGlobalRef(asset_mgr);
        //        globalAssetMgr = AAssetManager_fromJava(env, globalJavaAssetMgr);
        }

    // get from the orientation from the cached asset_mgr
    globalAssetMgr = AAssetManager_fromJava(env, globalJavaAssetMgr);
    auto cachedCfg = AConfiguration_new();
    AConfiguration_fromAssetManager(cachedCfg, globalAssetMgr);
    auto orientationFromCache = AConfiguration_getOrientation(cachedCfg);
    AConfiguration_delete(cachedCfg);

    // get the orientation from the fresh asset_mgr
    auto assetManager = AAssetManager_fromJava(env, asset_mgr);
    auto cfg = AConfiguration_new();
    AConfiguration_fromAssetManager(cfg, assetManager);
    auto orientation = AConfiguration_getOrientation(cfg);
    LOGI("GA-Orientation: orientation(%d) and orientationFromCache(%d) in %s", orientation, orientationFromCache, __FUNCTION__);
    AConfiguration_delete(cfg);
}
