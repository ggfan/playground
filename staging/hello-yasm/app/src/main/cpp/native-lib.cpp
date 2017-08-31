#include <jni.h>
#include <string>

extern "C" int64_t _add(int64_t a, int64_t b);

extern "C"
JNIEXPORT jstring JNICALL
Java_com_google_sample_hello_1yasm_YasmMainActivity_stringFromJNI(
    JNIEnv *env,
    jobject /* this */) {
  int64_t   val = _add(15, 20);
  std::string hello = "Hello from C++" + std::to_string(val);
  return env->NewStringUTF(hello.c_str());
}

