package com.ggfan.android.preview_view

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.camera.core.Camera
import androidx.camera.core.CameraSelector
import androidx.camera.core.Preview
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.view.PreviewView
import androidx.core.content.ContextCompat
import androidx.lifecycle.LifecycleOwner
import com.google.common.util.concurrent.ListenableFuture

class MainActivity : AppCompatActivity() {
    private lateinit var cameraProviderFuture : ListenableFuture<ProcessCameraProvider>
    private lateinit var previewView1: PreviewView
    private lateinit var previewView2: PreviewView
    private lateinit var camera: Camera
    private lateinit var permission:RuntimePermissions

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        permission = RuntimePermissions(this)
        cameraProviderFuture = ProcessCameraProvider.getInstance(this)
        previewView1 = findViewById(R.id.view_finder1)
        previewView2 = findViewById(R.id.view_finder2)

        if (!permission.hasPermissions()) {
            this.activityResultLauncher.launch(RuntimePermissions.PERMISSIONS_REQUIRED)
        } else {
            startPreview()
        }
    }

    override fun onStart() {
        super.onStart()
    }

    private fun startPreview() {
        cameraProviderFuture.addListener(Runnable {
            val cameraProvider = cameraProviderFuture.get()
            bindPreview(cameraProvider)
        }, ContextCompat.getMainExecutor(this))
    }
    private fun bindPreview(cameraProvider : ProcessCameraProvider) {
        var preview1 : Preview = Preview.Builder()
            .build()

        var preview2:Preview = Preview.Builder().build()

        var cameraSelector : CameraSelector = CameraSelector.Builder()
            .requireLensFacing(CameraSelector.LENS_FACING_BACK)
            .build()

        preview1.setSurfaceProvider(previewView1.getSurfaceProvider())
        preview2.setSurfaceProvider(previewView2.getSurfaceProvider())

        camera = cameraProvider.bindToLifecycle(this as LifecycleOwner,
            cameraSelector,
            preview1, preview2)
    }

    val activityResultLauncher = registerForActivityResult(ActivityResultContracts.RequestMultiplePermissions())
    { permissions ->
        // Handle Permission granted/rejected
        val permissionGranted = permissions.entries.filter {
            it.key in RuntimePermissions.PERMISSIONS_REQUIRED
        }.all {
            it.value
        }
        when (permissionGranted) {
            true -> startPreview()
            else -> Toast.makeText(this, "Permission request denied", Toast.LENGTH_LONG).show()
        }
    }
}