#include <jni.h>
#include <string>

extern "C" JNIEXPORT int32_t (startLib)(void);

// The function name is exported, fixed.
JNIEXPORT int32_t startLib(void){
  static int32_t count = 0;
  return count++;
}