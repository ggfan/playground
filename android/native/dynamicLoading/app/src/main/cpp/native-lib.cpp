#include <jni.h>
#include <string>
#include <android/log.h>
#include <dlfcn.h>

const char*    libName = "libPlugin.so";
const char*    entryFunc = "startLib";

void kickStartPlugin(void);

extern "C" JNIEXPORT jstring JNICALL
Java_com_android_example_two_1libs_MainActivity_nativeInit(
        JNIEnv* env,
        jobject /* this */) {
    kickStartPlugin();
    return env->NewStringUTF("success");
}

// global variables
extern "C" typedef int32_t (*PSTARTLIB)(void);
static void* pluginHandle = nullptr;
PSTARTLIB pluginEntryFunc = nullptr;

void kickStartPlugin(void) {
    if (pluginHandle) return;

    pluginHandle = dlopen(libName, RTLD_LAZY);
    pluginEntryFunc = reinterpret_cast<PSTARTLIB>(dlsym(pluginHandle, entryFunc));
    auto count = pluginEntryFunc();
    __android_log_print(ANDROID_LOG_DEBUG, "LibLoadingPOC", "result:  %d", count);
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_android_example_two_1libs_MainActivity_nativeHandleOnClick(JNIEnv *env, jobject thiz) {
  if (!pluginEntryFunc) return JNI_FALSE;

  auto count = pluginEntryFunc();
  __android_log_print(ANDROID_LOG_DEBUG, "LibLoadingPOC", "result:  %d", count);
  return JNI_TRUE;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_example_two_1libs_MainActivity_nativeClose(JNIEnv *env, jobject thiz) {
    if (pluginHandle) {
        dlclose(pluginHandle);
        pluginHandle = nullptr;
        pluginEntryFunc = nullptr;
    }
}