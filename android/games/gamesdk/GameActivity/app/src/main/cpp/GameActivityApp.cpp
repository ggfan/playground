#include <jni.h>
#include <game-activity/native_app_glue/android_native_app_glue.h>

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_gameactivity_MainActivity_startEngine(JNIEnv *env, jobject thiz) {
    // TODO: implement startEngine()
    return 0;
}

extern "C" {
    void android_main(struct android_app* app);
}

class NativeEngine;

static void handle_command_proxy(struct android_app* app, int32_t cmd);
class NativeEngine {
  public:
    NativeEngine(struct android_app* app) {
        mAndroidApp = app;
        bAnimating = false;
    }

    void GameLoop(void) {
        mAndroidApp->userData = this;
        mAndroidApp->onAppCmd = ::handle_command_proxy;
        mAndroidApp->textInputState = 0;

        while (1) {
            int events;
            struct android_poll_source* source;

            // If not animating, block until we get an event;
            // If animating, don't block.
            while ((ALooper_pollAll(IsAnimating() ? 0 : -1, NULL, &events,
                                    (void **) &source)) >= 0) {
                if (source != NULL) {
                    source->process(mAndroidApp, source);
                }
                if (mAndroidApp->destroyRequested) {
                    return;
                }
            }
            if (IsAnimating()) {
                DoFrame();
            }
        }
    }

    android_app* GetAndroidApp(void) { return mAndroidApp; }

    void DoFrame() {
        // todo: draw a frame of gfx
    }

    bool IsAnimating() {
        return  bAnimating;
    }

  private:
    struct android_app  *mAndroidApp;
    bool bAnimating = false;

};
void android_main(struct android_app* app) {
    NativeEngine *engine = new NativeEngine(app);
    app->userData = engine;
    engine->GameLoop();
    delete engine;
}

void handle_command_proxy(struct android_app* app, int32_t cmd) {
auto* engine = (NativeEngine*)(app->userData);
switch (cmd) {
case APP_CMD_SAVE_STATE:
// The system has asked us to save our current state.  Do so.
engine->GetAndroidApp()->savedState = nullptr;
// malloc(sizeof(struct saved_state));
//            *((struct saved_state*)engine->GetAndroidApp()->savedState) = engine->state;
//            engine->GetAndroidApp()->savedStateSize = sizeof(struct saved_state);
break;
case APP_CMD_INIT_WINDOW:
// The window is being shown, get it ready.
/*            if (engine->->window != nullptr) {
                engine_init_display(engine);
                engine_draw_frame(engine);
                }
*/
break;
case APP_CMD_TERM_WINDOW:
// The window is being hidden or closed, clean it up.
//            engine_term_display(engine);
break;
case APP_CMD_GAINED_FOCUS:
// When our app gains focus, we start monitoring the accelerometer.
/*
         if (engine->accelerometerSensor != nullptr) {
              ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                             engine->accelerometerSensor);
              // We'd like to get 60 events per second (in us).
              ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                             engine->accelerometerSensor,
                                             (1000L/60)*1000);
          }
*/
break;
case APP_CMD_LOST_FOCUS:
// When our app loses focus, we stop monitoring the accelerometer.
// This is to avoid consuming battery while not being used.
/*            if (engine->accelerometerSensor != nullptr) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
  */
break;
default:
break;
}
}
