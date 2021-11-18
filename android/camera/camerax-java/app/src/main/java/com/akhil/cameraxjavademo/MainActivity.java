package com.akhil.cameraxjavademo;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;
import androidx.camera.camera2.interop.Camera2CameraInfo;
import androidx.camera.core.Camera;
import androidx.camera.core.CameraInfo;
import androidx.camera.core.CameraSelector;
import androidx.camera.core.ImageAnalysis;
import androidx.camera.core.ImageCapture;
import androidx.camera.core.ImageCaptureException;
import androidx.camera.core.ImageProxy;
import androidx.camera.core.Preview;
// import androidx.camera.extensions.HdrImageCaptureExtender;
import androidx.camera.extensions.ExtensionMode;
import androidx.camera.extensions.ExtensionsManager;
import androidx.camera.lifecycle.ProcessCameraProvider;
import androidx.camera.view.PreviewView;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.lifecycle.LifecycleOwner;

import android.annotation.SuppressLint;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraCharacteristics;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.Toast;

import com.google.common.util.concurrent.ListenableFuture;

import java.io.File;
import java.lang.reflect.Array;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.Date;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Locale;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;

public class MainActivity extends AppCompatActivity {
    private final String TAG = "camerax-java-demo";

    private Executor executor = Executors.newSingleThreadExecutor();
    private int REQUEST_CODE_PERMISSIONS = 1001;
    private final String[] REQUIRED_PERMISSIONS = new String[]{"android.permission.CAMERA", "android.permission.WRITE_EXTERNAL_STORAGE"};

    PreviewView mPreviewView;
    ImageView captureImage;

    private int frameCount = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mPreviewView = findViewById(R.id.previewView);
        captureImage = findViewById(R.id.captureImg);

        if(allPermissionsGranted()){
            startCamera(); //start camera if permission has been granted by user
        } else{
            ActivityCompat.requestPermissions(this, REQUIRED_PERMISSIONS, REQUEST_CODE_PERMISSIONS);
        }
    }

    private void startCamera() {

        final ListenableFuture<ProcessCameraProvider> cameraProviderFuture = ProcessCameraProvider.getInstance(this);

        cameraProviderFuture.addListener(new Runnable() {
            @Override
            public void run() {
                try {
                    // copy the following code into the documenation
                    ProcessCameraProvider cameraProvider = cameraProviderFuture.get();
                    bindPreview(cameraProvider);

                } catch (ExecutionException | InterruptedException e) {
                    // No errors need to be handled for this Future.
                    // This should never be reached.
                }
            }
        }, ContextCompat.getMainExecutor(this));
    }

    @SuppressLint({"UnsafeOptInUsageError"})
    Boolean isBackCameraLevel3Device(ProcessCameraProvider cameraProvider) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N)
            return Boolean.FALSE;

        int hardwareLevel = -1;

        @SuppressLint("RestrictedApi") List<CameraInfo> cameraInfos = CameraSelector.DEFAULT_BACK_CAMERA
                .filter(cameraProvider.getAvailableCameraInfos());

        if(!cameraInfos.isEmpty()) {
            hardwareLevel = Camera2CameraInfo
                    .from(cameraInfos.get(0))
                    .getCameraCharacteristic(CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL);
        }
        return (hardwareLevel == CameraCharacteristics.INFO_SUPPORTED_HARDWARE_LEVEL_3);
    }
    void bindPreview(@NonNull ProcessCameraProvider cameraProvider) throws ExecutionException, InterruptedException {

        Log.i(TAG, "Is Level 3 Camera: " + isBackCameraLevel3Device(cameraProvider).toString());
        Preview preview = new Preview.Builder()
                .build();

        CameraSelector cameraSelector = new CameraSelector.Builder()
                .requireLensFacing(CameraSelector.LENS_FACING_BACK)
                .build();

        //Vendor-Extensions (The CameraX extensions dependency in build.gradle)
            /*
                ExtensionMode.FACE_RETOUCH;
                ExtensionMode.BOKEH;
                ExtensionMode.AUTO;
                ExtensionMode.NONE;
                ExtensionMode.HDR;
             */
        int mode = ExtensionMode.NIGHT;
        ExtensionsManager extensionsManager = ExtensionsManager.getInstance(this).get();
        if(extensionsManager != null &&
           extensionsManager.isExtensionAvailable(cameraProvider, cameraSelector, mode))
        {
            cameraSelector = extensionsManager.getExtensionEnabledCameraSelector(cameraProvider, cameraSelector, mode);
            Log.i(TAG, "Extension " + mode + " is supported");
        } else {
            Log.i(TAG, "Extension " + mode + " is not supported");
        }

        ImageAnalysis imageAnalysis = new ImageAnalysis.Builder()
                .build();
        imageAnalysis.setAnalyzer(executor, new ImageAnalysis.Analyzer() {
            int frameCount = 0;
            long timeStamp  = System.currentTimeMillis();
            @Override
            public void analyze(@NonNull ImageProxy imageProxy) {
                frameCount++;
                if (frameCount == 100) {
                    long time = timeStamp;
                    timeStamp = System.currentTimeMillis();
                    Log.i(TAG, "analyzer frameRate = " + (frameCount * 1000.0f / (timeStamp - time)));
                    frameCount = 0;
                }
                imageProxy.close();
            }
        });

        ImageCapture.Builder builder = new ImageCapture.Builder();

        final ImageCapture imageCapture = builder
                .setTargetRotation(this.getWindowManager().getDefaultDisplay().getRotation())
                .build();

        preview.setSurfaceProvider(mPreviewView.getSurfaceProvider());

        Camera camera = null;
        try {
            cameraProvider.bindToLifecycle((LifecycleOwner)this, cameraSelector, preview, imageCapture, imageAnalysis);
        } catch (Exception e) {
            Log.e(TAG, e.getLocalizedMessage());
        }

        captureImage.setOnClickListener(v -> {

            SimpleDateFormat mDateFormat = new SimpleDateFormat("yyyyMMddHHmmss", Locale.US);
            String name = mDateFormat.format(new Date()) + ".jpg";
            File file = new File(getBatchDirectoryName(), name);

            Log.i(this.TAG, "capturing image to: " +  name);
            ImageCapture.OutputFileOptions outputFileOptions = new ImageCapture.OutputFileOptions.Builder(file).build();
            imageCapture.takePicture(outputFileOptions, executor, new ImageCapture.OnImageSavedCallback () {
                @Override
                public void onImageSaved(@NonNull ImageCapture.OutputFileResults outputFileResults) {
                    new Handler(Looper.getMainLooper()).post(new Runnable() {
                        @Override
                        public void run() {
                            Toast.makeText(MainActivity.this, "Image Saved successfully", Toast.LENGTH_SHORT).show();
                        }
                    });
                }
                @Override
                public void onError(@NonNull ImageCaptureException error) {
                    error.printStackTrace();
                }
            });
        });
    }

    public String getBatchDirectoryName() {

        String app_folder_path = "";
        app_folder_path = Environment.getExternalStorageDirectory().toString() + "/images";
        File dir = new File(app_folder_path);
        if (!dir.exists() && !dir.mkdirs()) {

        }

        return app_folder_path;
    }

    private boolean allPermissionsGranted(){

        for(String permission : REQUIRED_PERMISSIONS){
            if(ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED){
                return false;
            }
        }
        return true;
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {

        if(requestCode == REQUEST_CODE_PERMISSIONS){
            if(allPermissionsGranted()){
                startCamera();
            } else{
                Toast.makeText(this, "Permissions not granted by the user.", Toast.LENGTH_SHORT).show();
                this.finish();
            }
        }
    }
}
