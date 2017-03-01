/*
 * Copyright (C) 2017 The Android Open Source Project
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
 */
#include <utility>
#include <stdint.h>
#include <unistd.h>
#include <cinttypes>

#include <camera/NdkCameraManager.h>
#include <camera/NdkCameraMetadata.h>
#include "camera_manager.h"
#include "utils/native_debug.h"
#include "camera_utils.h"

/*
 * CameraManager callbacks: this sample does not handle hotplug,
 * does nothing in callback
 */
void OnCameraAvailable(void* ctx, const char* id) {
    reinterpret_cast<NativeCamera*>(ctx)->OnCameraStatusChanged(id, true);
}
void OnCameraUnavailable(void* ctx, const char* id){
    reinterpret_cast<NativeCamera*>(ctx)->OnCameraStatusChanged(id, false);
}

/*
 * CameraDevice callbacks
 */
void OnDeviceStateChanges(void* ctx, ACameraDevice* dev) {
    reinterpret_cast<NativeCamera*>(ctx)->OnDeviceStateChanges(dev);
}
void OnDeviceErrorChanges(void* ctx, ACameraDevice* dev, int err) {
    reinterpret_cast<NativeCamera*>(ctx)->OnDeviceErrorChanges(dev, err);
}

// CaptureSession creation callbacks, pass through into mgr object
void OnSessionClosed(void* ctx, ACameraCaptureSession* ses) {
    reinterpret_cast<NativeCamera*>(ctx)->OnSessionStateChange(
            ses, CaptureSessionState::CLOSED);
}
void OnSessionReady(void* ctx, ACameraCaptureSession* ses) {
    reinterpret_cast<NativeCamera*>(ctx)->OnSessionStateChange(
            ses, CaptureSessionState::READY);
}
void OnSessionActive(void* ctx, ACameraCaptureSession *ses) {
    reinterpret_cast<NativeCamera*>(ctx)->OnSessionStateChange(
            ses, CaptureSessionState::ACTIVE);
}

NativeCamera::NativeCamera(ANativeWindow* win) :
                  nativeWindow_(win),
                  cameraMgr_(nullptr),
                  activeCameraId_(""),
                  outputContainer_(nullptr),
                  captureSessionState_(CaptureSessionState::MAX_STATE) {

    cameras_.clear();
    cameraMgr_ = ACameraManager_create();
    ASSERT(cameraMgr_, "Failed to create cameraManager");

    // Pick up a back-facing camera to preview
    EnumerateCamera();
    ASSERT(activeCameraId_.size(), "Unknown ActiveCameraIdx");

    mgrListener_ =  {
      .context = this,
      .onCameraAvailable = ::OnCameraAvailable,
      .onCameraUnavailable = ::OnCameraUnavailable,
    };
    CALL_MGR(registerAvailabilityCallback(cameraMgr_,
                                          &mgrListener_));

    // Create camera device for the selected Camera device
    cameraListener_ = {
        .context = this,
        .onDisconnected = ::OnDeviceStateChanges,
        .onError = ::OnDeviceErrorChanges,
    };
    CALL_MGR(openCamera(cameraMgr_, activeCameraId_.c_str(),
                        &cameraListener_, &cameras_[activeCameraId_].device_));

    // Create output from this app's ANativeWindow, and add into output container
    ANativeWindow_acquire(nativeWindow_);
    CALL_CONTAINER(create(&outputContainer_));
    CALL_OUTPUT(create(nativeWindow_, &output_));
    CALL_CONTAINER(create(&outputContainer_));
    CALL_CONTAINER(add(outputContainer_, output_));

    // Create output target from the same ANativeWindow, add it into captureRequest
    CALL_TARGET(create(nativeWindow_, &outputTarget_));
    CALL_DEV(createCaptureRequest(cameras_[activeCameraId_].device_,
                                  TEMPLATE_PREVIEW, &captureRequest_));
    CALL_REQUEST(addTarget(captureRequest_, outputTarget_));

    // Create a capture session for the given preview request
    sessionListener_ = {
            .context = this,
            .onActive = ::OnSessionActive,
            .onReady = ::OnSessionReady,
            .onClosed = ::OnSessionClosed,
    };
    captureSessionState_ = CaptureSessionState::READY;
    CALL_DEV(createCaptureSession(cameras_[activeCameraId_].device_,
                                  outputContainer_, &sessionListener_,
                                  &captureSession_));
}

NativeCamera::~NativeCamera() {

    ACameraCaptureSession_close(captureSession_);
    CALL_REQUEST(removeTarget(captureRequest_, outputTarget_));
    ACaptureRequest_free(captureRequest_);
    ACameraOutputTarget_free(outputTarget_);

    CALL_CONTAINER(remove(outputContainer_, output_));
    ACaptureSessionOutputContainer_free(outputContainer_);
    ACaptureSessionOutput_free(output_);

    ANativeWindow_release(nativeWindow_);

    for (auto &cam: cameras_) {
        if (cam.second.device_) {
            CALL_DEV(close(cam.second.device_));
        }
    }
    cameras_.clear();
    if (cameraMgr_) {
        CALL_MGR(unregisterAvailabilityCallback(cameraMgr_,
                                                &mgrListener_));
        ACameraManager_delete(cameraMgr_);
        cameraMgr_ = nullptr;
    }
}

/*
 * EnumerateCamera()
 *     Loop through cameras on the system, pick up
 *     1) back facing one if available
 *     2) otherwise pick the first one reported to us
 */
void NativeCamera::EnumerateCamera() {
    ACameraIdList*  cameraIds = nullptr;
    CALL_MGR(getCameraIdList(cameraMgr_, &cameraIds));

    for (int i = 0; i < cameraIds->numCameras; ++i) {
        const char* id = cameraIds->cameraIds[i];

        ACameraMetadata *metadataObj;
        CALL_MGR(getCameraCharacteristics(cameraMgr_, id, &metadataObj));

        int32_t count = 0;
        const uint32_t* tags = nullptr;
        ACameraMetadata_getAllTags(metadataObj, &count, &tags);
        for (int tagIdx=0; tagIdx < count; ++tagIdx) {
            if (ACAMERA_LENS_FACING == tags[tagIdx]) {
                ACameraMetadata_const_entry lensInfo = {0,};
                CALL_METADATA(getConstEntry(metadataObj,tags[tagIdx], &lensInfo));
                CameraId cam(id);
                cam.facing_ = static_cast<acamera_metadata_enum_android_lens_facing_t>(lensInfo.data.u8[0]);
                cam.owner_ = false;
                cam.device_ = nullptr;
                cameras_[cam.id_] = cam;
                if (cam.facing_ == ACAMERA_LENS_FACING_BACK) {
                    activeCameraId_ = cam.id_;
                }
                break;
            }
        }
        ACameraMetadata_free(metadataObj);
    }

    ASSERT(cameras_.size(), "No Camera Available on the device");
    if (activeCameraId_.length() == 0) {
        // if no back facing camera found, pick up the first one to use...
        activeCameraId_ = cameras_.begin()->second.id_;
    }
    ACameraManager_deleteCameraIdList(cameraIds);
}

/*
 * OnCameraStatusChanged()
 *  handles Callback from ACameraManager
 */
void NativeCamera::OnCameraStatusChanged(const char* id, bool available) {

    cameras_[std::string(id)].available_ = available ? true : false;
}

/*
 * OnDeviceStateChanges()
 *   callback from CameraDevice called when device is disconnected
 */
void NativeCamera::OnDeviceStateChanges(ACameraDevice* dev) {
    std::string id(ACameraDevice_getId(dev));
    LOGI("device %s is disconnected", id.c_str());

    cameras_[id].available_ = false;
    ACameraDevice_close(cameras_[id].device_);
    cameras_.erase(id);
}

void NativeCamera::OnDeviceErrorChanges(ACameraDevice* dev, int err) {
    std::string id(ACameraDevice_getId(dev));
    CameraId *cameraInfo = nullptr;

    LOGI("CameraDevice %s is in error %#x", id.c_str(), err);
    PrintCameraDeviceError(err);

    CameraId &cam = cameras_[id];

    switch (err) {
        case ERROR_CAMERA_IN_USE:
            cam.available_ = false;
            cam.owner_ = false;
            break;
        case ERROR_CAMERA_SERVICE:
        case ERROR_CAMERA_DEVICE:
        case ERROR_CAMERA_DISABLED:
        case ERROR_MAX_CAMERAS_IN_USE:
            cam.available_ = false;
            cam.owner_ = false;
            break;
        default:
            LOGI("Unknown Camera Device Error: %#x", err);
    }
}
/*
 * OnSessionStateChange():
 *   Handles Capture Session callbacks
 */
void NativeCamera::OnSessionStateChange(ACameraCaptureSession* ses,
                                        CaptureSessionState state) {
    ASSERT(ses && ses == captureSession_, "CaptureSession from camera is null");
    if (state == captureSessionState_ &&
        captureSessionState_ != CaptureSessionState::MAX_STATE) {
        LOGW("Duplicate State (%d) for session (%p), ignored", state, ses);
        return;
    }

    captureSessionState_ = state;
    if (state == CaptureSessionState::READY) {
        LOGI("Capture Session is complete: Moving it to display queue");
        //TODO: move it into display, and sechedule another capture
        return;
    }
    if (state == CaptureSessionState::ACTIVE) {
        LOGI("Capture Session just started");
        return;
    }
    if (state == CaptureSessionState::CLOSED) {
        LOGI("Session is closed");
        // we have cleaned up everything externaly,
        // Nothing else to do
    }
}
/*
 * Animate()
 *   Toggle preview start/stop
 */
void NativeCamera::Animate(void) {
    if (captureSessionState_ == CaptureSessionState::READY) {
      CALL_SESSION(setRepeatingRequest(captureSession_,
                                       nullptr,
                                       1,
                                       &captureRequest_,
                                       nullptr));
    } else if (captureSessionState_ == CaptureSessionState::ACTIVE) {
        ACameraCaptureSession_stopRepeating(captureSession_);
    }
}
