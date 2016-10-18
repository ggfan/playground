#include <jni.h>
#include <string>
#include <cassert>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// global variables
struct AudioInfo {
    SLObjectItf  engineObjItf_;
    SLEngineItf  engineItf_;
    SLObjectItf  outMixObjItf_;

    SLObjectItf  uriPlayerObj_;
    SLPlayItf    uriPlayItf_;

    JNIEnv*  env_;
    jobject  jniObj_;
    std::string uri_;

    explicit AudioInfo(JNIEnv* env, jobject instance):
        env_(env),
        jniObj_(instance),
        engineObjItf_ (nullptr),
        engineItf_ (nullptr) {
        createEngine();
    }

    ~AudioInfo() {
        (*uriPlayerObj_)->Destroy(uriPlayerObj_);
        (*outMixObjItf_)->Destroy(outMixObjItf_);
        (*engineObjItf_) -> Destroy(engineObjItf_);
    }
    void createEngine(void)
    {
        SLresult result;

        // create engine
        result = slCreateEngine(&engineObjItf_, 0, NULL, 0, NULL, NULL);
        assert(SL_RESULT_SUCCESS == result);

        // realize the engine
        result = (*engineObjItf_)->Realize(engineObjItf_, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);

        // get the engine interface, which is needed in order to create other objects
        result = (*engineObjItf_)->GetInterface(engineObjItf_, SL_IID_ENGINE, &engineItf_);
        assert(SL_RESULT_SUCCESS == result);

        result = (*engineItf_)->CreateOutputMix(engineItf_, &outMixObjItf_, 0, NULL, NULL);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;

        // realize the output mix
        result = (*outMixObjItf_)->Realize(outMixObjItf_, SL_BOOLEAN_FALSE);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;

    }

    int  createURIPlayer(const char * uri) {
        if (!uri)
            return 0;
        uri_ = std::string(uri);
        SLDataLocator_URI loc_uri = {SL_DATALOCATOR_URI, (SLchar*)uri};
        SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
        SLDataSource audioSrc = {&loc_uri, &format_mime};
        // configure audio sink
        SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outMixObjItf_};
        SLDataSink audioSnk = {&loc_outmix, NULL};

        // create audio player
        const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
        const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
        SLresult result = (*engineItf_)->CreateAudioPlayer(engineItf_, &uriPlayerObj_,
                                                           &audioSrc, &audioSnk, 3, ids, req);
        // note that an invalid URI is not detected here, but during prepare/prefetch on Android,
        // or possibly during Realize on other platforms
        assert(SL_RESULT_SUCCESS == result);

        // should we set this to async?  since we are calling from a UI thread
        result = (*uriPlayerObj_)->Realize(uriPlayerObj_, SL_BOOLEAN_FALSE);
        assert(result == SL_RESULT_SUCCESS);

        // get the play interface
        result = (*uriPlayerObj_)->GetInterface(uriPlayerObj_, SL_IID_PLAY, &uriPlayItf_);
        assert(SL_RESULT_SUCCESS == result);
        (void)result;

        // will not get any other controls
        return 1;
    }
};

AudioInfo* audioInfo = nullptr;

// helper functions
// create the engine and output mix objects

// Return not 0 : success
extern "C" JNIEXPORT jint JNICALL
Java_com_google_example_uriplayer_MainActivity_createURIPlayer(JNIEnv *env, jobject instance,
                                                               jstring uri_) {

    audioInfo = new AudioInfo(env, instance);
    assert( audioInfo);

    const char *uri = env->GetStringUTFChars(uri_, 0);
    int status = audioInfo->createURIPlayer(uri);
    env->ReleaseStringUTFChars(uri_, uri);

    return static_cast<jint>(status);
}

extern "C" JNIEXPORT jint JNICALL
Java_com_google_example_uriplayer_MainActivity_startURIPlayer(JNIEnv *env, jobject instance,
                                                              jboolean play) {
    assert(env == audioInfo->env_);
    assert(instance == audioInfo->jniObj_);

    SLresult  status =  (*audioInfo->uriPlayItf_)->SetPlayState(audioInfo->uriPlayItf_,
                                   (play == JNI_TRUE)? SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_PAUSED);
    assert(status == SL_RESULT_SUCCESS);
    return 1;
}

extern "C" JNIEXPORT void JNICALL
Java_com_google_example_uriplayer_MainActivity_deleteURIPlayer(JNIEnv *env, jobject instance) {

    // TODO
    assert(env == audioInfo->env_);
    assert(instance == audioInfo->jniObj_);
    delete audioInfo;
}

