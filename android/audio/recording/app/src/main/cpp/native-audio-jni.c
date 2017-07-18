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

/* Notes:
 *     To put the recording device into the fast path, it needs to be
 *        *) sample frequency:  native sample frequence that came from device
 *        *) no audio processing is required
 *     what does not matter:
 *       *) sample buffer size
 *       *) output device reverb request ( since this is for input stream, output device does not matter)
 * How to confirm:
 *    look at the logcat after createRecorder for:
 *      AudioFlinger: AudioFlinger's thread 0xe3883300 ready to run
        W AudioFlinger: createRecordTrack_l(): mismatch between requested flags (00000005) and input flags (00000001)
        I AudioRecord: AUDIO_INPUT_FLAG_FAST successful; frameCount 0  <==== this one matters
        I SoundTriggerHwService::Module: void android::SoundTriggerHwService::Module::onCallbackEvent(const sp<android::SoundTriggerHwService::CallbackEvent> &) mClient == 0
        E audio_route: unable to find path 'set-capture-format-default'
 */
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include <jni.h>



// for __android_log_print(ANDROID_LOG_INFO, "YourApp", "formatted message");
 #include <android/log.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>


// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
    SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

// recorder interfaces
static SLObjectItf recorderObject = NULL;
static SLRecordItf recorderRecord;
static SLAndroidSimpleBufferQueueItf recorderBufferQueue;

static const int32_t kBufCount = 2;
static int32_t samplesPerFrame = 0;
static int32_t channelCount = 1;
static int16_t* recordBuffers[] = {NULL, NULL};
static volatile int frameCount = 0;

#define  RECORD_ON  1
#define  RECORD_OFF 0
static volatile int recordingOn = RECORD_OFF;
static volatile int curBufIdx = 0;

// this callback handler is called every time a buffer finishes recording
void bqRecorderCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == recorderBufferQueue);
    assert(NULL == context);

    frameCount++;

    if(recordingOn) {
      int16_t* buf = recordBuffers[curBufIdx];
      memset(buf, 0, samplesPerFrame * channelCount * sizeof(int16_t));

      SLresult result;
      result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, buf,
                                               samplesPerFrame * channelCount * sizeof(int16_t));
      assert(SL_RESULT_SUCCESS == result);
    }
    curBufIdx = ++curBufIdx % kBufCount;
}

// create the engine and output mix objects
void Java_com_example_recording_NativeRecording_createEngine(JNIEnv* env, jclass clazz)
{
    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // create output mix, with environmental reverb specified as a non-required interface
    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, ids, req);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the environmental reverb interface
    // this could fail if the environmental reverb effect is not available,
    // either because the feature is not present, excessive CPU load, or
    // the required MODIFY_AUDIO_SETTINGS permission was not requested and granted
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB,
            &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(
                outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    // ignore unsuccessful result codes for environmental reverb, as it is optional for this example
}

// create audio recorder: recorder is not in fast path
//    like to avoid excessive re-sampling while playing back from Hello & Android clip
jboolean Java_com_example_recording_NativeRecording_createAudioRecorder(JNIEnv* env, jclass clazz, jint sampleRate, jint samplesPerBurst)
{
    SLresult result;

    // configure audio source
    SLDataLocator_IODevice loc_dev = {
        .locatorType = SL_DATALOCATOR_IODEVICE,
        .deviceType = SL_IODEVICE_AUDIOINPUT,
        .deviceID = SL_DEFAULTDEVICEID_AUDIOINPUT,
        .device = NULL
    };
    SLDataSource audioSrc = {
        .pLocator = &loc_dev,
        .pFormat = NULL
    };

    // configure audio sink
    SLDataLocator_AndroidSimpleBufferQueue loc_bq = {
        .locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        .numBuffers = 2
    };
    SLDataFormat_PCM format_pcm = {
        .formatType = SL_DATAFORMAT_PCM,
        .numChannels  = 1,     // channelCount
        /*
         * to use fast audio, sampleRate must be coming from the device native sample rate
         */
        .samplesPerSec  = sampleRate * 1000,
        .bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16,
        .containerSize = SL_PCMSAMPLEFORMAT_FIXED_16,
        .channelMask = SL_SPEAKER_FRONT_CENTER,
        .endianness = SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSink audioSnk = {
        .pLocator = &loc_bq,
        .pFormat = &format_pcm};

    // create audio recorder
    // (requires the RECORD_AUDIO permission)
    const SLInterfaceID id[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorderObject, &audioSrc,
            &audioSnk, 1, id, req);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }

    // realize the audio recorder
    result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
    if (SL_RESULT_SUCCESS != result) {
        return JNI_FALSE;
    }

    // get the record interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recorderRecord);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // get the buffer queue interface
    result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
            &recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    // register callback on the buffer queue
    result = (*recorderBufferQueue)->RegisterCallback(recorderBufferQueue, bqRecorderCallback,
            NULL);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;
    samplesPerFrame = samplesPerBurst;  // to be passed from caller
    for (int idx = 0; idx < kBufCount; idx++) {
      int32_t size = samplesPerFrame * channelCount * sizeof(int16_t);
      int16_t *buf = (int16_t*)malloc(size);
      memset(buf, 0, size);
      recordBuffers[idx] = buf;
    }
    return JNI_TRUE;
}


// set the recording state for the audio recorder
void Java_com_example_recording_NativeRecording_startRecording(JNIEnv* env, jclass clazz)
{
    if (recordingOn == RECORD_ON) {
      return;
    }

    SLresult result;

    // in case already recording, stop recording and clear buffer queue
    result = (*recorderRecord)->SetRecordState(recorderRecord,
                                               SL_RECORDSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
    result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // enqueue an empty buffer to be filled by the recorder
    // (for streaming recording, we would enqueue at least 2 empty buffers to start things off)
    for(int idx = 0; idx < kBufCount; idx++) {
      result = (*recorderBufferQueue)->Enqueue(recorderBufferQueue, recordBuffers[idx],
                                               samplesPerFrame * channelCount * sizeof(short));
      assert(SL_RESULT_SUCCESS == result);
    }
    // start recording
    curBufIdx = 0;
    recordingOn = RECORD_ON;
    frameCount = 0;
    result = (*recorderRecord)->SetRecordState(recorderRecord, SL_RECORDSTATE_RECORDING);
    assert(SL_RESULT_SUCCESS == result);
}

// set the recording state for the audio recorder
void Java_com_example_recording_NativeRecording_stopRecording(JNIEnv* env, jclass clazz)
{

    recordingOn = RECORD_OFF;

    SLresult result;
    result = (*recorderRecord)->SetRecordState(recorderRecord,
                                               SL_RECORDSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
    result = (*recorderBufferQueue)->Clear(recorderBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
    __android_log_print(ANDROID_LOG_INFO, "ANDROID_RECORD",
                        "Recorded FrameCount = %d", frameCount);

}

// shut down the native audio system
void Java_com_example_recording_NativeRecording_shutdown(JNIEnv* env, jclass clazz)
{
    // destroy audio recorder object, and invalidate all associated interfaces
    if (recorderObject != NULL) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recorderRecord = NULL;
        recorderBufferQueue = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
        outputMixEnvironmentalReverb = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

    for (int idx = 0; idx < kBufCount; idx++) {
      free(recordBuffers[idx]);
      recordBuffers[idx] = NULL;
    }
}
