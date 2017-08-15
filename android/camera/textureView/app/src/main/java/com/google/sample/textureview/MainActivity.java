package com.google.sample.textureview;

import java.io.IOException;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.CameraInfo;
import android.os.Bundle;

import android.view.Gravity;
import android.view.Menu;
import android.view.Surface;
import android.view.TextureView;
import android.view.TextureView.SurfaceTextureListener;
import android.widget.FrameLayout;

public class MainActivity extends Activity implements SurfaceTextureListener {
    private TextureView myTexture;
    private Camera mCamera;
    int cameraId;

    @SuppressLint("NewApi")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        myTexture = new TextureView(this);
        myTexture.setSurfaceTextureListener(this);
        setContentView(myTexture);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        // getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @SuppressLint("NewApi")
    @Override
    public void onSurfaceTextureAvailable(SurfaceTexture arg0, int arg1, int arg2) {
        cameraId = -1;
        int numberOfCameras = Camera.getNumberOfCameras();
        for (int i = 0; i < numberOfCameras; i++) {
            CameraInfo info = new CameraInfo();
            Camera.getCameraInfo(i, info);
            if (info.facing == CameraInfo.CAMERA_FACING_BACK) {
                cameraId = i;
                break;
            }
        }
        mCamera = Camera.open(cameraId);
        Camera.Size previewSize = mCamera.getParameters().getPreviewSize();

        myTexture.setLayoutParams(new FrameLayout.LayoutParams(
                previewSize.width, previewSize.height, Gravity.CENTER));

        try {
            mCamera.setPreviewTexture(arg0);
        } catch (IOException t) {
        }

        CameraInfo info = new CameraInfo();
        mCamera.getCameraInfo(cameraId, info);
        int  displayRotation = getCorrectCameraOrientation(info, mCamera);
        myTexture.setAlpha(1.0f);
        myTexture.setRotation(displayRotation);

        // This could not be set as it will destroy the aspect ratio
        // it tends to stretch the image makes it double wide or narrow
        //   mCamera.setDisplayOrientation(displayRotation);

        mCamera.startPreview();

    }
    public int getCorrectCameraOrientation(CameraInfo info, Camera camera) {

        int rotation = this.getWindowManager().getDefaultDisplay().getRotation();
        int degrees = 0;

        switch(rotation){
            case Surface.ROTATION_0:
                degrees = 0;
                break;

            case Surface.ROTATION_90:
                degrees = 90;
                break;

            case Surface.ROTATION_180:
                degrees = 180;
                break;

            case Surface.ROTATION_270:
                degrees = 270;
                break;

        }

        int result;
        if(info.facing==Camera.CameraInfo.CAMERA_FACING_FRONT){
            result = (info.orientation + degrees) % 360;
            result = (360 - result) % 360;
        }else{
            result = (info.orientation - degrees + 360) % 360;
        }

        return result;
    }
    @Override
    public boolean onSurfaceTextureDestroyed(SurfaceTexture arg0) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    @Override
    public void onSurfaceTextureSizeChanged(SurfaceTexture arg0, int arg1,
                                            int arg2) {
        // TODO Auto-generated method stub
    }

    @Override
    public void onSurfaceTextureUpdated(SurfaceTexture arg0) {
        // TODO Auto-generated method stub
    }
}