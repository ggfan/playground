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
import android.util.Rational
import android.widget.SeekBar
import android.widget.Toast
import androidx.camera.core.*
import androidx.camera.lifecycle.ProcessCameraProvider
import com.google.android.preview.databinding.ActivityMainBinding
import java.util.*
import java.lang.IllegalArgumentException

import com.gfan.android.camerax_lib.CameraSelectorAdapter
import java.util.concurrent.atomic.AtomicInteger

/**
 * ToDO: https://github.com/devadvance/circularseekbar
 */
class MainActivity : AppCompatActivity() {
    private lateinit var uiBinding: ActivityMainBinding
    private var imageCapture: ImageCapture? = null
    private lateinit var outputDirectory:File
    private lateinit var cameraExecutor: ExecutorService
    private lateinit var camera:Camera

    private var exposureIndexValue = 0
    private var exposureStep = Rational(0, 1)
    private var exposureAdjustments = AtomicInteger(0)

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        uiBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(uiBinding.root)
        // Request camera permissions
        if (allPermissionsGranted()) {
            startCamera()
        } else {
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS,
                REQUEST_CODE_PERMISSIONS)
        }

        // Set up the listener for take photo button
        uiBinding.cameraCaptureButton.setOnClickListener {
            if (exposureAdjustments.get() == 0) takePhoto() }
        outputDirectory = getOutputDirectory()
        cameraExecutor = Executors.newSingleThreadExecutor()

    }

    private fun initExposureValueControl() {
        if (this::camera.isInitialized) {
            val evState = camera.cameraInfo.exposureState
            uiBinding.evSeekBar.apply {
                isEnabled = evState.isExposureCompensationSupported
                max = evState.exposureCompensationRange.upper
                min = evState.exposureCompensationRange.lower
                exposureIndexValue = evState.exposureCompensationIndex
                exposureStep = evState.exposureCompensationStep
                progress = exposureIndexValue

                /**
                 * The exposureIndex setting is about 100 ~ 200ms; if there is a cancelling event
                 * needed, it might takes up the 2 setting cycles (like 300 ms) (from samsung fold3)
                 * a few things:
                 *     -- the numbers are fairly large when dynamically updating the seekBar
                 *     -- it is not feasible to turn on/off the UI with every index update, too annoying
                 *     -- in the listener, the current hardware setting needs to read out
                 *     -- In the application, the setting index has to be serialized?
                 */
                setOnSeekBarChangeListener(object: SeekBar.OnSeekBarChangeListener {
                    override fun onProgressChanged(seekBar: SeekBar?, progress: Int, fromUser: Boolean)
                    {
                        if (!(this@MainActivity::camera.isInitialized)
                            || seekBar == null || !fromUser) {
                                return
                        }

                        exposureAdjustments.incrementAndGet()
                        val startTime = System.currentTimeMillis()
                        camera.cameraControl.setExposureCompensationIndex(progress)
                            .addListener({
                                val endTime = System.currentTimeMillis()
                                exposureIndexValue = camera.cameraInfo.exposureState.exposureCompensationIndex
                                Log.i(TAG, "Set $progress, current: $exposureIndexValue" +
                                    " executionTime:  ${endTime - startTime}")
                                exposureAdjustments.decrementAndGet()
                            }, mainExecutor)
                    }

                    override fun onStartTrackingTouch(p0: SeekBar?) {
                    }
                    override fun onStopTrackingTouch(p0: SeekBar?) {
                    }
                })

                Log.i(TAG, "ExposureRange: $min-$max, progress:$progress")
            }
        }

    }
    private fun takePhoto() {
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
        uiBinding.evSeekBar.isEnabled = false
        val startTime = System.currentTimeMillis()
        imageCapture.takePicture(
            outputOptions,
            ContextCompat.getMainExecutor(this),
            object : ImageCapture.OnImageSavedCallback {
                override fun onError(exc: ImageCaptureException) {
                    Log.e(TAG, "Photo capture failed: ${exc.message}", exc)
                    uiBinding.evSeekBar.isEnabled = false
                }

                override fun onImageSaved(output: ImageCapture.OutputFileResults) {
                    val endTime = System.currentTimeMillis()

                    val savedUri = Uri.fromFile(photoFile)
                    val msg = "Photo capture succeeded: $savedUri in ${endTime - startTime}ms"
                    Toast.makeText(baseContext, msg, Toast.LENGTH_SHORT).show()
                    Log.d(TAG, msg)
                    uiBinding.evSeekBar.isEnabled = true
                }
            })
    }

    private fun startCamera() {
        val cameraProviderFuture = ProcessCameraProvider.getInstance(this)

        cameraProviderFuture.addListener ({
            // Used to bind the lifecycle of cameras to the lifecycle owner
            val cameraProvider = cameraProviderFuture.get()

            // Preview
            val preview = Preview.Builder()
                .build()
                .also {
                    it.setSurfaceProvider(uiBinding.viewFinder.surfaceProvider)
                }

            imageCapture = ImageCapture.Builder().build()
            val cameraSelector:CameraSelector =
                CameraSelectorAdapter.selectExternalOrBestCamera(cameraProvider)
                    ?: throw IllegalArgumentException()
            try {
                // Unbind use cases before rebinding
                cameraProvider.unbindAll()

                // Bind use cases to camera
                camera = cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageCapture)

            } catch(exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
            }

            initExposureValueControl()
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

    companion object {
        private const val TAG = "CameraXCodelab"
        private const val FILENAME_FORMAT="yyyy-MM-dd-HH-mm-ss-SSS"
        private const val   REQUEST_CODE_PERMISSIONS = 10
        private val REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)
    }

}
