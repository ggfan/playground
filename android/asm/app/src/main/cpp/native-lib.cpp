#include <jni.h>
#include <string>

int asm_add(int i, int j);

extern "C"
JNIEXPORT jstring JNICALL
Java_com_google_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
  std::string hello = "Hello from C++";
  int value  = asm_add(10, 5);
  return env->NewStringUTF(hello.c_str());
}



int asm_add(int i, int j)
{
  int res = 0;
  __asm (
  // .altmacro

  "ADD %[result], %[input_i], %[input_j]"
  : [result] "=r" (res)
  : [input_i] "r" (i), [input_j] "r" (j)
  );
  return res;
}