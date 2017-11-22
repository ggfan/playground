#include <jni.h>
#include <string>
#include <assert.h>
#include <include/NeuralNetworks.h>

extern "C"
JNIEXPORT jstring JNICALL
Java_com_google_nnapi_basic_MainActivity_stringFromJNI(
    JNIEnv *env,
    jobject /* this */) {

  if (initNNAPI() < 0) {
    assert(0);
  }

  ANeuralNetworksInitialize();
  ANeuralNetworksShutdown();

  std::string hello = "Hello from C++";
  return env->NewStringUTF(hello.c_str());
}
