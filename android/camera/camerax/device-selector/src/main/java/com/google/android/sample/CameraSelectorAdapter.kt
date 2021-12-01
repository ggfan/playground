package com.google.android.sample

import android.annotation.SuppressLint
import android.hardware.camera2.CameraCharacteristics
import android.util.Log
import androidx.camera.camera2.interop.Camera2CameraInfo
import androidx.camera.camera2.interop.ExperimentalCamera2Interop
import androidx.camera.core.CameraInfo
import androidx.camera.core.CameraSelector
import androidx.camera.lifecycle.ProcessCameraProvider

class CameraSelectorAdapter {
    @SuppressLint("UnsafeOptInUsageError")
    @androidx.annotation.OptIn(ExperimentalCamera2Interop::class)
    companion object {
        fun selectExternalOrBestCamera(provider: ProcessCameraProvider):CameraSelector? {
            val cam2Infos = provider.availableCameraInfos.map {
                Camera2CameraInfo.from(it)
            }.sortedByDescending {
                it.getCameraCharacteristic(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL)
            }
            return when {
                cam2Infos.isNotEmpty() ->
                {
                    CameraSelector.Builder()
                        .addCameraFilter {
                            it.filter { camInfo ->
                                val thisCamId = Camera2CameraInfo.from(camInfo).cameraId
                                thisCamId == cam2Infos[0].cameraId
                            }
                        }.build()
                }
                else -> null
            }
        }

        fun selectExternalCamera(provider:ProcessCameraProvider) : CameraSelector? {
            val cameraSelector = CameraSelector.Builder()
                .addCameraFilter {
                    it.filter { camInfo ->
                        val level = Camera2CameraInfo.from(camInfo)
                        .getCameraCharacteristic(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL)
                        val lensFacing = Camera2CameraInfo.from(camInfo)
                        .getCameraCharacteristic(CameraCharacteristics.LENS_FACING)
                        Log.i(TAG, "=====Hardware Level  $level, LENS_FACING: $lensFacing")

                        level == CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_EXTERNAL
                    }
                }.build()
            return cameraSelector
        }

        fun selectBestCamera2(provider: ProcessCameraProvider?):CameraSelector? {
            val cam2Infos = provider?.availableCameraInfos?.map {
                Camera2CameraInfo.from(it)
            }
            var maxLevel: Int = -1
            var candidateCam2Info: Camera2CameraInfo? = null
            cam2Infos?.forEach {
                val level =
                    it.getCameraCharacteristic(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL)
                if (level != null && level > maxLevel) {
                    maxLevel = level
                    candidateCam2Info = it
                }
            }
            // build camera selector with the best CameraID with CameraInfo's cameraSelector.
            if (maxLevel != -1) {
                val selector = provider!!.availableCameraInfos.find {
                            Camera2CameraInfo.from(it).cameraId == candidateCam2Info?.cameraId
                        }?.cameraSelector
                Log.i(TAG, "==== built camera selector: $selector?.toString()")
                return selector
            }
            return null
        }
        fun selectDefaultBackCamera() : CameraSelector? {
            return CameraSelector.DEFAULT_BACK_CAMERA
        }
        fun selectDefaultFrontCamera() : CameraSelector? {
            return CameraSelector.DEFAULT_FRONT_CAMERA
        }
    }
}

val TAG="CameraxPlayground"
