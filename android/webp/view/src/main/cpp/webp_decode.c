#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <webp/decode.h>
#include <android/log.h>
#include <errno.h>
#include "webp_decode.h"

int decode_file(const char* in_file, ANativeWindow_Buffer* frameBuf, AAssetManager *assetMgr) {
    uint8_t *buf;
    int32_t len;

    AAsset* frameFile = AAssetManager_open(assetMgr, in_file, AASSET_MODE_BUFFER);
    if (frameFile == NULL) {
        __android_log_print(ANDROID_LOG_ERROR,"decode_file",
                            "open asset error: %d, %s",
                            errno, strerror(errno));
        assert(0);
        return -1;
    }
    len = AAsset_getLength(frameFile);
    buf = (uint8_t*) malloc(len);
    assert(buf);
    len = AAsset_read(frameFile, buf, len);
    assert(len > 0);
    AAsset_close(frameFile);

    WebPDecoderConfig config;
    VP8StatusCode  status;
    if (!WebPInitDecoderConfig(&config)) {
        assert(0);
        free(buf);  //too many of those error checking code
        return -1;
    }

    // If I do not use the input features, I do not have to call this function
    // check it out later
    status = WebPGetFeatures(buf, len, &config.input);
    if ( status != VP8_STATUS_OK) {
        assert(0);
        free(buf);
        return -1;
    }

    //let's decode it into a buffer ...
    int bytePerPixel = 2;
    config.options.bypass_filtering = 1;
    config.options.no_fancy_upsampling = 1;
    config.options.flip = 0;
    config.options.use_scaling = 1;
    config.options.scaled_width = frameBuf->width;
    config.options.scaled_height = frameBuf->height;

    //this does not seems to be working for android.
    config.options.use_threads = 1;

    switch (frameBuf->format) {
        case WINDOW_FORMAT_RGB_565:
            config.output.colorspace = MODE_RGB_565;
            bytePerPixel = 2;
            break;
        case WINDOW_FORMAT_RGBA_8888:
        case WINDOW_FORMAT_RGBX_8888:
            config.output.colorspace = MODE_RGBA;
            bytePerPixel = 4;
            break;
        default:
            assert( 0 );
            return -1;
    }
    config.output.width = frameBuf->width;
    config.output.height = frameBuf->height;
    config.output.is_external_memory = 1;
    config.output.private_memory = frameBuf->bits;
    config.output.u.RGBA.stride = frameBuf->stride * bytePerPixel;
    config.output.u.RGBA.rgba  = frameBuf->bits;
    config.output.u.RGBA.size  = config.output.height * config.output.u.RGBA.stride;


    status = WebPDecode(buf, len, &config);
    if (status != VP8_STATUS_OK) {
        assert(0);
        free(buf);
        return -1;
    }

    WebPFreeDecBuffer(&config.output);
    free(buf);
    return 1;
}