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
import android.widget.Button
import android.widget.SeekBar
import android.widget.TextView
import android.widget.Toast
import androidx.camera.core.*
import androidx.camera.extensions.ExtensionMode
import androidx.camera.extensions.ExtensionsManager
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.lifecycle.LifecycleRegistry
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.example.android.camera.utils.GenericListAdapter
import com.google.android.preview.databinding.ActivityMainBinding
import java.util.*
import java.lang.IllegalArgumentException

import com.gfan.android.camerax_lib.*
import kotlinx.coroutines.launch

import java.util.concurrent.atomic.AtomicInteger


/**
 * ToDO: https://github.com/devadvance/circularseekbar
 */
class PreviewActivity : AppCompatActivity() {
    private lateinit var uiBinding: ActivityMainBinding
    private var imageCapture: ImageCapture? = null
    private lateinit var outputDirectory:File
    private lateinit var cameraExecutor: ExecutorService
    private lateinit var camera:Camera
    private lateinit var baseCameraSelector:CameraSelector

    private var lifecycleRegistry : LifecycleRegistry? = null

    private var extensionSelectorIndex = 0
    private var exposureIndexValue = 0
    private var exposureStep = Rational(0, 1)
    private var exposureAdjustments = AtomicInteger(0)


    private var extensionCapabilities = listOf<Int>()

    private var cameraIndex = 0

    override fun getLifecycle(): LifecycleRegistry {
        if (lifecycleRegistry == null) {
            lifecycleRegistry = LifecycleRegistry(this)
        }
        return lifecycleRegistry!!
    }

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

        uiBinding.cameraButton.setOnClickListener {
            val types = CameraSelectorAdapter.supportedCameraSelectorTypes()
            cameraIndex = ++cameraIndex % types.size
            startCamera()
            (it as Button).text = types[cameraIndex]
        }

        // Set up the listener for take photo button
        uiBinding.captureButton.setOnClickListener {
            // Taking photos while exposure adjustment is not in progress
            if (exposureAdjustments.get() == 0) takePhoto()
        }
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
                        if (!(this@PreviewActivity::camera.isInitialized)
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
            val cameraName = CameraSelectorAdapter.supportedCameraSelectorTypes()[cameraIndex]
            var cameraSelector:CameraSelector =
                CameraSelectorAdapter.selectCamera(cameraName, cameraProvider)
                    ?: throw IllegalArgumentException()

            baseCameraSelector = cameraSelector

            // experimenting the extensions
            val extMgr = ExtensionsManager.getInstanceAsync(this, cameraProvider).get()
            extensionCapabilities = ExtensionMode_getSupportedModes().filter {
                    extMgr.isExtensionAvailable(cameraSelector, it)
            }
            initializeExtensions()

            val extType = extensionCapabilities[extensionSelectorIndex]
            if(extMgr.isExtensionAvailable(cameraSelector, extType)) {
                cameraSelector = extMgr.getExtensionEnabledCameraSelector(cameraSelector, extType)
                Log.i(TAG, "===== Turning extension Type: $extType")
            } else  {
                Log.i(TAG, "===== Extension $extType is not supported")
            }

            // extensions end
            try {
                // Unbind use cases before rebinding
                cameraProvider.unbindAll()

                // Bind use cases to camera
                camera = cameraProvider.bindToLifecycle(this, cameraSelector, preview, imageCapture)
                initExposureValueControl()

                uiBinding.featureStatus.text = ExtensionMode_getName(extType)
            } catch(exc: Exception) {
                Log.e(TAG, "Use case binding failed", exc)
                uiBinding.featureStatus.text = ExtensionMode_getName(ExtensionMode.NONE)
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

    companion object {
        private const val TAG = "CameraXCodelab"
        private const val FILENAME_FORMAT="yyyy-MM-dd-HH-mm-ss-SSS"
        private const val   REQUEST_CODE_PERMISSIONS = 10
        private val REQUIRED_PERMISSIONS = arrayOf(Manifest.permission.CAMERA)
    }


    /**
     *  initializeQualitySectionsUI():
     *    Populate a RecyclerView to display camera capabilities:
     *       - one front facing
     *       - one back facing
     *    User selection is saved to qualitySelectorIndex, will be used
     *    in the bindCaptureUsecase().
     */
    private fun initializeExtensions() {
        val selectorStrings = extensionCapabilities.map {
            ExtensionMode_getName(it)
        }

        // create the adapter to QualitySelector RecyclerView
        uiBinding.extensions.apply {
            layoutManager = LinearLayoutManager(this@PreviewActivity)
            adapter = GenericListAdapter(
                selectorStrings,
                itemLayoutId = R.layout.single_extension_view
            ) { holderView, qcString, position ->

                holderView.apply {
                    findViewById<TextView>(R.id.extensionTextView)?.text = qcString
                    // select the default quality selector
                    isSelected = (position == extensionSelectorIndex)
                }

                holderView.setOnClickListener { view ->
                    if (extensionSelectorIndex == position) return@setOnClickListener

                    uiBinding.extensions.let {
                        // deselect the previous selection on UI.
                        it.findViewHolderForAdapterPosition(extensionSelectorIndex)
                            ?.itemView
                            ?.isSelected = false
                    }
                    // turn on the new selection on UI.
                    view.isSelected = true
                    extensionSelectorIndex = position

                    // rebind the use cases to put the new QualitySelection in action.
                     lifecycleScope.launch {
                        startCamera()
                    }
                }
            }
            isEnabled = false
        }
    }

}
