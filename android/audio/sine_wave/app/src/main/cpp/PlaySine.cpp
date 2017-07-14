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

/* This is a JNI example where we use native methods to play sounds
 * using OpenSL ES. See the corresponding Java source file located at:
 *
 *   src/com/example/sinewave/NativeAudio/NativeAudio.java
 */

#include <assert.h>
#include <jni.h>
#include <string.h>
#include <pthread.h>
#include <atomic>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "SineGenerator.h"

// pre-recorded sound clips, both are 8 kHz mono 16-bit signed little endian
static const char hello[] =
#include "hello_clip.h"
;

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLEffectSendItf bqPlayerEffectSend;
static SLVolumeItf bqPlayerVolume;
static SLmilliHertz bqPlayerSampleRate = 0;

// aux effect on the output mix, used by the buffer queue player
static const SLEnvironmentalReverbSettings reverbSettings =
    SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

const  int kBufCount = 2;
int16_t *buffers[kBufCount] = {nullptr, nullptr};
uint32_t bufSizeInByte = 0;
uint32_t samplesPerBuf = 0;
uint32_t curBufIdx = 0;
int32_t  kChannelCount = 1;

class SineGenerator*   sineGenerator = nullptr;
std::atomic<bool>  playToneOn;

// this callback handler is called every time a buffer finishes playing
void bqPlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);

    int16_t* buf = buffers[curBufIdx];
    curBufIdx = (curBufIdx + 1) % kBufCount;
    if (playToneOn) {
        sineGenerator->render(buf, kChannelCount, samplesPerBuf);
    } else {
      memset(buf, 0, bufSizeInByte);
    }
    SLresult result;
    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                             buf, bufSizeInByte);
    assert(SL_RESULT_SUCCESS == result);
}


// create the engine and output mix objects
extern "C" JNIEXPORT void JNICALL Java_com_example_sinewave_NativeAudio_createEngine(JNIEnv* env, jclass clazz)
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


// create buffer queue audio player
extern "C" JNIEXPORT void JNICALL Java_com_example_sinewave_NativeAudio_createBufferQueueAudioPlayer(JNIEnv* env,
        jclass clazz, jint sampleRate, jint bufSize)
{
    SLresult result;
    assert(sampleRate);

    bqPlayerSampleRate = sampleRate * 1000;

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
        .locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        .numBuffers = 2 };
    SLDataFormat_PCM format_pcm = {
        .formatType = SL_DATAFORMAT_PCM,
        .numChannels = 1,
        .samplesPerSec = bqPlayerSampleRate,
        .bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16,
        .containerSize = SL_PCMSAMPLEFORMAT_FIXED_16,
        .channelMask = SL_SPEAKER_FRONT_CENTER,
        .endianness = SL_BYTEORDER_LITTLEENDIAN,
    };
   SLDataSource audioSrc = {
       .pLocator = &loc_bufq,
       .pFormat = &format_pcm
   };

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {
        .locatorType = SL_DATALOCATOR_OUTPUTMIX,
        .outputMix = outputMixObject
    };
    SLDataSink audioSnk = {
        .pLocator = &loc_outmix,
        .pFormat = NULL
    };

    /*
     * create audio player:
     *     fast audio does not support when SL_IID_EFFECTSEND is required, skip it
     *     for fast audio case
     */
    const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE,
                                  SL_IID_VOLUME,
                                  SL_IID_EFFECTSEND,
                                    /*SL_IID_MUTESOLO,*/};
    const SLboolean req[3] = {
        SL_BOOLEAN_TRUE,
        SL_BOOLEAN_TRUE,
        SL_BOOLEAN_TRUE,
        /*SL_BOOLEAN_TRUE,*/ };

    result = (*engineEngine)->CreateAudioPlayer(engineEngine,
                                                &bqPlayerObject,
                                                &audioSrc,
                                                &audioSnk,
                                                bqPlayerSampleRate? 2 : 3,
                                                ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bqPlayerCallback, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // get the effect send interface
    bqPlayerEffectSend = NULL;
    if( 0 == bqPlayerSampleRate) {
        result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_EFFECTSEND,
                                                 &bqPlayerEffectSend);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;
    }

    // get the volume interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume);
    assert(SL_RESULT_SUCCESS == result);

    sineGenerator = new SineGenerator();
    sineGenerator->setup(440, sampleRate, .25);

    // Create 2 buffers wit silent audio
    samplesPerBuf = bufSize;
    bufSizeInByte = bufSize * 1 * sizeof(int16_t);
    for(int idx = 0; idx < kBufCount; idx++) {
        // hard coded to 16 bit audio, 1 channel
        buffers[idx] = new int16_t[bufSize * 1];
        assert(buffers[idx]);
        memset(buffers[idx], 0, samplesPerBuf * 1 * sizeof(int16_t));

        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue,
                                                 buffers[idx],
                                                 bufSizeInByte);
        assert(result == SL_RESULT_SUCCESS);
    }
    curBufIdx = 0;
    playToneOn = false;

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
}


// select the desired clip and play count, and enqueue the first buffer if idle
extern "C" JNIEXPORT jboolean JNICALL Java_com_example_sinewave_NativeAudio_PlayAudio(JNIEnv* env, jclass clazz,
        jint count)
{
    playToneOn = !playToneOn;
    return JNI_TRUE;
}


// shut down the native audio system
extern "C" JNIEXPORT void JNICALL Java_com_example_sinewave_NativeAudio_shutdown(JNIEnv* env, jclass clazz)
{
   // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerEffectSend = NULL;
        bqPlayerVolume = NULL;
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

    if(sineGenerator) {
        delete sineGenerator;
        sineGenerator = nullptr;
    }
  for(auto& p: buffers) {
    delete [] p;
    p = nullptr;
  }

}

extern "C" JNIEXPORT void JNICALL
Java_com_example_sinewave_NativeAudio_StopAudio(JNIEnv *env, jclass type) {
}