#include <jni.h>
#include <string>
#include <android/log.h>
#include <game-text-input/gametextinput.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_android_example_gametextinputjava_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello GameTextInput";
    return env->NewStringUTF(hello.c_str());
}

static GameTextInput *gameTextInputObj = nullptr;
static const uint32_t  MAX_TEXT_INPUT_LENGTH = 1024;
extern "C"
JNIEXPORT jboolean JNICALL
Java_com_android_example_gametextinputjava_MainActivity_initNativeTextInput(JNIEnv *env, jobject thiz) {
    if(gameTextInputObj) {
      GameTextInput_destroy(gameTextInputObj);
    }
    gameTextInputObj = GameTextInput_init(env, MAX_TEXT_INPUT_LENGTH);
    return gameTextInputObj ? JNI_TRUE  : JNI_FALSE;
}


extern "C"
JNIEXPORT jboolean JNICALL
Java_com_android_example_gametextinputjava_MainActivity_terminateNativeTextInput(JNIEnv *env,
                                                                                 jobject thiz) {
  if (gameTextInputObj) {
      GameTextInput_destroy(gameTextInputObj);
  }
  gameTextInputObj = nullptr;
  return JNI_TRUE;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_android_example_gametextinputjava_InputEnabledTextView_onTextInputEventNative(JNIEnv *env,
                                                                                       jobject thiz,
                                                                                       jobject soft_keyboard_event) {
    GameTextInput_processEvent(gameTextInputObj, soft_keyboard_event);
}

// processing the event which including text (input from user).
extern "C" void onEventCallback(
        void *context, const GameTextInputState *current_state) {
    if (!context || !current_state) return;

    __android_log_print(ANDROID_LOG_INFO, "onEventCallback",
                        "message: %s", current_state->text_UTF8);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_android_example_gametextinputjava_MainActivity_setInputConnectionNative(JNIEnv *env,
                                                                                 jobject thiz,
                                                                                 jobject connection) {
    // TODO: implement setInputConnectionNative()
    GameTextInput_setInputConnection(gameTextInputObj, connection);
//    GameTextInput_setEventCallback(gameTextInputObj, onEventCallback, env);
    GameTextInput_setEventCallback(gameTextInputObj, [] (void* ctx, const GameTextInputState *state) {
        if (!ctx || !state) return;

        __android_log_print(ANDROID_LOG_INFO, "GameTextInput",
                            "message: %s", state->text_UTF8);

        }, env);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_android_example_gametextinputjava_MainActivity_showIme(JNIEnv *env, jobject thiz) {
    if (!gameTextInputObj)  return;
    GameTextInput_showIme(gameTextInputObj, 0);
}