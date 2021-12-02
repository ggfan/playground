package com.google.android.preview

import android.content.pm.PackageManager
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import androidx.camera.core.ImageCapture
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import java.io.File
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import android.Manifest
import android.annotation.SuppressLint
import android.icu.text.SimpleDateFormat
import android.net.Uri
import android.util.Log
import android.widget.Toast
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import com.google.android.preview.databinding.ActivityMainBinding
import java.nio.ByteBuffer
import java.util.*
import kotlin.system.measureTimeMillis

import androidx.camera.video.*
import java.lang.IllegalArgumentException
import androidx.camera.video.VideoCapture as VideoCaptureX

import com.gfan.android.camerax_lib.CameraSelectorAdapter
typealias LumaListener = (luma: Double) -> Unit


class MainActivity : AppCompatActivity() {
    private lateinit var uiBinding: ActivityMainBinding
    private var imageCapture: ImageCapture? = null
    private lateinit var outputDirectory:File
    private lateinit var cameraExecutor: ExecutorService
    private lateinit var recorder: Recorder
    private lateinit var videoCapturer: VideoCaptureX<Recorder>
    private var activeRecorder: Recording? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        uiBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(uiBinding.root)
        // setContentView(R.layout.activity_main)
        // Request camera permissions
        if (allPermissionsGranted()) {
            startCamera()
        } else {
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS,
                REQUEST_CODE_PERMISSIONS)
        }

        // Set up the listener for take photo button
        uiBinding.cameraCaptureButton.setOnClickListener { takePhoto() }
        outputDirectory = getOutputDirectory()
        cameraExecutor = Executors.newSingleThreadExecutor()
    }
    private fun takePhoto() {
        val recordInstance = activeRecorder
        if (recordInstance != null) {
            recordInstance.stop()
            activeRecorder = null

            Log.i("====Capture", "Stopped!")
            return
        }
        // Create time-stamped output file to hold the image
        val photoFile = File(
            outputDirectory,
            SimpleDateFormat(FILENAME_FORMAT, Locale.US
            ).format(System.currentTimeMillis()) + ".3pg")

        // Create output options object which contains file + metadata
        val fileOptions = FileOutputOptions.Builder(photoFile).build()
        activeRecorder = recorder
            .prepareRecording(this, fileOptions)
            .start(cameraExecutor, {
                when (it) {
                    is VideoRecordEvent.Start -> {
                        Log.i(TAG, "Recording Started")
                        uiBinding.cameraCaptureButton.text = "Stop Capture"
                    }
                    is VideoRecordEvent.Status -> Log.i(TAG, "Intermediate Status Report")
                    is VideoRecordEvent.Resume -> Log.i(TAG, "Recording just resumed")
                    is VideoRecordEvent.Pause -> Log.i(TAG, "Recording just paused")
                    is VideoRecordEvent.Finalize -> {
                        val event: VideoRecordEvent.Finalize = it
                        Log.i(
                            TAG,
                            "Recording just finished, status(${event.hasError()}), size(${event.outputResults.outputUri}"
                        )
                        uiBinding.cameraCaptureButton.text= "Start Capture"
                    }
                    else -> Log.e(TAG, "Unknown event type $it")
                }
                Log.i(TAG, "Status: size(${it.recordingStats.numBytesRecorded})," +
                        "time(${it.recordingStats.recordedDurationNanos}/100000000000L)")
            })
    }

    private fun takePhoto2() {
        // Get a stable reference of the modifiable image capture use case
        val imageCapture = imageCapture ?: return

        // Create time-stamped output file to hold the image
        val photoFile = File(
            outputDirectory,
            SimpleDateFormat(FILENAME_FORMAT, Locale.US
            ).format(System.currentTimeMillis()) + ".jpg")

        // Create output options object which contains file + metadata
        val outputOptions = ImageCapture.OutputFileOptions.Builder(photoFile).build()

        // Set up image capture listener, which is triggered after photo has
        // been taken
        imageCapture.takePicture(
            outputOptions, ContextCompat.getMainExecutor(this), object : ImageCapture.OnImageSavedCallback {
                override fun onError(exc: ImageCaptureException) {
                    Log.e(TAG, "Photo capture failed: ${exc.message}", exc)
                }

                override fun onImageSaved(output: ImageCapture.OutputFileResults) {
                    val savedUri = Uri.fromFile(photoFile)
                    val msg = "Photo capture succeeded: $savedUri"
                    Toast.makeText(baseContext, msg, Toast.LENGTH_SHORT).show()
                    Log.d(TAG, msg)
                }
            })
    }

    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(this)

        cameraProviderFuture.addListener(Runnable {
            // Used to bind the lifecycle of cameras to the lifecycle owner
            val cameraProvider: ProcessCameraProvider = cameraProviderFuture.get()

            // Preview
            val preview = Preview.Builder()
                .build()
                .also {
                    it.setSurfaceProvider(uiBinding.viewFinder1.surfaceProvider)
                }

            Log.i(TAG, "====ViewPort: ")

            val preview2 = Preview.Builder()
                .build()
                .also {
                    it.setSurfaceProvider(uiBinding.viewFinder2.surfaceProvider)
                }
            imageCapture = ImageCapture.Builder().build()

            val imageAnalyzer = ImageAnalysis.Builder()
                //.setBackpressureStrategy(STRATEGY_BLOCK_PRODUCER)
                .setOutputImageFormat(ImageAnalysis.OUTPUT_IMAGE_FORMAT_RGBA_8888)
                .build()
                .also {
                    it.setAnalyzer(cameraExecutor,
                        LuminosityAnalyzer {
                            luma ->
                        Log.d(TAG, "Average luminosity: $luma")
                    })
                }
            // Select a Camera with CameraSelectorAdapter
            val cameraSelector:CameraSelector = CameraSelectorAdapter.selectExternalOrBestCamera(cameraProvider) ?: throw IllegalArgumentException()

            // create capture
            recorder = Recorder.Builder().setExecutor(cameraExecutor)
                .build()
            videoCapturer = VideoCaptureX.withOutput<Recorder>(recorder)


            try {
                // Unbind use cases before rebinding
                cameraProvider.unbindAll()

                // Bind use cases to camera
                // cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageCapture, imageAnalyzer, videoCapturer)
                cameraProvider.bindToLifecycle(this, cameraSelector, preview)

            } catch(exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
            }

        }, ContextCompat.getMainExecutor(this))
    }
    @SuppressLint("MissingSuperCall")
    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<String>, grantResults:
        IntArray) {
        if (requestCode == REQUEST_CODE_PERMISSIONS) {
            if (allPermissionsGranted()) {
                startCamera()
            } else {
                Toast.makeText(this,
                    "Permissions not granted by the user.",
                    Toast.LENGTH_SHORT).show()
                finish()
            }
        }
    }

    private fun allPermissionsGranted() = REQUIRED_PERMISSIONS.all {
        ContextCompat.checkSelfPermission(baseContext, it) == PackageManager
            .PERMISSION_GRANTED
    }

    private fun getOutputDirectory(): File {
        val mediaDir = externalMediaDirs.firstOrNull()?.let {
            File(it, resources.getString(R.string.app_name)).apply { mkdirs() }
        }
        return if (mediaDir != null && mediaDir.exists()) mediaDir else filesDir
    }

    override fun onDestroy() {
        cameraExecutor.shutdown()
        super.onDestroy()
    }

    private class LuminosityAnalyzer(private val listener: LumaListener) : ImageAnalysis.Analyzer {

        private fun ByteBuffer.toByteArray(): ByteArray {
            rewind()    // Rewind the buffer to zero
            val data = ByteArray(remaining())
            get(data)   // Copy the buffer into a byte array
            return data // Return the byte array
        }

        override fun analyze(image: ImageProxy) {
            val timeInMillis = measureTimeMillis {

                val buffer = image.planes[0].buffer
                var copy = ByteBuffer.allocate(buffer.capacity())
                buffer.rewind()
                copy.put(buffer)
                buffer.rewind()
                image.close()

                val data = copy.toByteArray()
                val pixels = data.map { it.toInt() and 0xFF }
                val luma = pixels.average()
                // val luma : Double = .5;
                listener(luma)
            }
            Log.d("Analyzer Function","===Execution Time: $timeInMillis")
        }
    }

    companion object {
        private const val TAG = "CameraXCodelab"
        private const val FILENAME_FORMAT="yyyy-MM-dd-HH-mm-ss-SSS"
        private const val   REQUEST_CODE_PERMISSIONS = 10
        private val REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)
    }
}
