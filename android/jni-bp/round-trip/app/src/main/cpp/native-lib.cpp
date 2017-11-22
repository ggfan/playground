#include <android/log.h>
#include <jni.h>
#include <string>
#include <chrono>
#include <time.h>

// from android samples
/* return current time in milliseconds */
static long now_ns(void) {

  struct timespec res;
  clock_gettime(CLOCK_REALTIME, &res);
  return 1000000000 * res.tv_sec + res.tv_nsec;
}

extern "C"
JNIEXPORT jint
JNICALL
Java_com_google_example_round_1trip_MainActivity_jniInc(
        JNIEnv *env,
        jobject /* this */,
        jint seed) {
  return (seed+1);
}

int testCall(int seed) {
  return (seed+1);
}
extern "C"
JNIEXPORT jlong JNICALL
Java_com_google_example_round_1trip_MainActivity_getPureNativeIncTime(JNIEnv *env, jobject instance,
                                                             jint iterations) {

  long time = 0;
  int val = 0;
  volatile bool  result = false;
  for (jint i = 0; i  < iterations; i++) {
    auto startTime = now_ns();
    val = testCall(val);
    time += now_ns() - startTime;
  }
  time /= iterations;
  __android_log_print(ANDROID_LOG_INFO, "Rount-trip",
                      "iteration = %d, time = %d", iterations, (int)time);
  return time;
}

struct Parameters {
    jint   int1_;
    jlong  long1_;
    jint   int2_;
    jlong  long2_;
    jint   int3_;
    jlong  long3_;
    jfloat float1_;
    jfloat float2_;
    jfloat float3_;
    jchar  type_;
};

struct fieldSignature {
    char* name_;
    char* jniType_;
    jfieldID   fieldID_;
};

enum FIELD_IDS {
    INT1   = 0,
    LONG1,
    INT2,
    LONG2,
    INT3,
    LONG3,
    FLOAT1,
    FLOAT2,
    FLOAT3,
    TYPE,
    ID_COUNT
};

fieldSignature parameterSignatures[] = {
        { "int1_", "I", nullptr },
        { "long1_", "J", nullptr },
        { "int2_", "I", nullptr },
        { "long2_", "J", nullptr },
        { "int3_", "I", nullptr },
        { "long3_", "J", nullptr },
        { "float1_", "F", nullptr },
        { "float2_", "F", nullptr },
        { "float3_", "F", nullptr },
        { "type_", "C", nullptr },
};
bool fieldIdCacheValid = false;

Parameters appData;


extern "C"
JNIEXPORT void JNICALL
Java_com_google_example_round_1trip_MainActivity_passByObject(JNIEnv *env, jobject instance,
                                                              jobject parameterObj, jboolean fieldCache) {
  if (!fieldCache) {
    jfieldID fieldId;

    jclass className = env->GetObjectClass(parameterObj);
    fieldId = env->GetFieldID(className, "int1_", "I");
    appData.int1_ = env->GetIntField(parameterObj, fieldId);

    fieldId = env->GetFieldID(className, "int2_", "I");
    appData.int2_ = env->GetIntField(parameterObj, fieldId);

    fieldId = env->GetFieldID(className, "int3_", "I");
    appData.int3_ = env->GetIntField(parameterObj, fieldId);

    fieldId = env->GetFieldID(className, "long1_", "J");
    appData.long1_ = env->GetLongField(parameterObj, fieldId);

    fieldId = env->GetFieldID(className, "long2_", "J");
    appData.long2_ = env->GetLongField(parameterObj, fieldId);

    fieldId = env->GetFieldID(className, "long3_", "J");
    appData.long3_ = env->GetLongField(parameterObj, fieldId);

    fieldId = env->GetFieldID(className, "float1_", "F");
    appData.float1_ = env->GetFloatField(parameterObj, fieldId);
    fieldId = env->GetFieldID(className, "float2_", "F");
    appData.float2_ = env->GetFloatField(parameterObj, fieldId);
    fieldId = env->GetFieldID(className, "float3_", "F");
    appData.float3_ = env->GetFloatField(parameterObj, fieldId);

    fieldId = env->GetFieldID(className, "type_", "C");
    appData.type_ = env->GetCharField(parameterObj, fieldId);
    return;
  }

  // using field cache...
  if (!fieldIdCacheValid) {
    jclass className = env->GetObjectClass(parameterObj);
    for (uint32_t idx = FIELD_IDS::INT1; idx < FIELD_IDS::ID_COUNT; idx++) {
      parameterSignatures[idx].fieldID_ = env->GetFieldID(className,
                                                          parameterSignatures[idx].name_,
                                                          parameterSignatures[idx].jniType_);
    }
    fieldIdCacheValid = true;
  }
  appData.int1_ = env->GetIntField(parameterObj,
                                   parameterSignatures[FIELD_IDS::INT1].fieldID_);
  appData.int2_ = env->GetIntField(parameterObj,
                                   parameterSignatures[FIELD_IDS::INT2].fieldID_);
  appData.int3_ = env->GetIntField(parameterObj,
                                   parameterSignatures[FIELD_IDS::INT3].fieldID_);

  appData.long1_ = env->GetLongField(parameterObj,
                                     parameterSignatures[FIELD_IDS::LONG1].fieldID_);
  appData.long2_ = env->GetLongField(parameterObj,
                                     parameterSignatures[FIELD_IDS::LONG2].fieldID_);
  appData.long3_ = env->GetLongField(parameterObj,
                                     parameterSignatures[FIELD_IDS::LONG3].fieldID_);

  appData.float1_ = env->GetFloatField(parameterObj,
                                       parameterSignatures[FIELD_IDS::FLOAT1].fieldID_);
  appData.float2_ = env->GetFloatField(parameterObj,
                                       parameterSignatures[FIELD_IDS::FLOAT2].fieldID_);
  appData.float3_ = env->GetFloatField(parameterObj,
                                       parameterSignatures[FIELD_IDS::FLOAT3].fieldID_);
  appData.type_ = env->GetCharField(parameterObj,
                                    parameterSignatures[FIELD_IDS::TYPE].fieldID_);

}

extern "C"
JNIEXPORT void JNICALL
Java_com_google_example_round_1trip_MainActivity_passByValue(JNIEnv *env, jobject instance,
                                                             jint intVal1, jint intVal2,
                                                             jint intVal3, jlong longVal1,
                                                             jlong longVal2, jlong longVal3,
                                                             jfloat floatVal1, jfloat floatVal2,
                                                             jfloat floatVal3, jchar type) {
  appData.int1_ = intVal1;
  appData.int2_ = intVal2;
  appData.int3_ = intVal3;

  appData.long1_ = longVal1;
  appData.long2_ = longVal2;
  appData.long3_ = longVal3;

  appData.float1_ = floatVal1;
  appData.float2_ = floatVal2;
  appData.float3_ = floatVal3;

  appData.type_ = type;

}
