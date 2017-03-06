package com.sample.surfaceview;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Canvas;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.Display;
import android.view.Gravity;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.FrameLayout;

import junit.framework.Assert;

public class ViewActivity extends Activity
		implements TextureView.SurfaceTextureListener,
		ActivityCompat.OnRequestPermissionsResultCallback {
	private  TextureView textureView_;
	private  int width_, height_;
	private  boolean  cameraGranted_ = false;
    private  boolean  startPending_ = false;
	Surface  surface_ = null;
	private  long nativeCamera_ = 0;
	private  int cameraWidth_;
	private  int cameraHeight_;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        Display display = getWindowManager().getDefaultDisplay();
		height_ = display.getMode().getPhysicalHeight();
		width_ = display.getMode().getPhysicalWidth();
		RequestCamera();
	}

	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
		Log.i("Camera-Sample", "SurfaceTexture Available");
		surface.setDefaultBufferSize(cameraWidth_, cameraHeight_);
		surface_ = new Surface(surface);
		if (cameraGranted_) {
			Log.i("Camera-Sample", "Starting Preview");
			notifySurfaceTextureCreated(surface_);
		} else {
			startPending_ = true;
		}

	}

	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
		// Notify native side after surface has been created
		if (surface_ != null) {
		    notifySurfaceTextureChanged(surface_);
	    }
	}

	public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
		notifySurfaceTextureDestroyed(surface_);
		surface_ = null;
		return true;
	}

	public void onSurfaceTextureUpdated(SurfaceTexture surface) {
		// Invoked every time there's a new Camera preview frame
		// this is place to call NDK drawing things

		Log.i("Sample", "calling the frames");
	}

	private static final int PERMISSION_REQUEST_CODE_CAMERA = 1;
	public void RequestCamera() {
		if (ActivityCompat.checkSelfPermission(this, Manifest.permission.CAMERA) !=
				PackageManager.PERMISSION_GRANTED) {
			ActivityCompat.requestPermissions(
					this,
					new String[] { Manifest.permission.CAMERA },
					PERMISSION_REQUEST_CODE_CAMERA);
			return;
		}
		cameraGranted_ = true;
		CreateCameraPreviewEngine();
	}

	@Override
	public void onRequestPermissionsResult(int requestCode,
										   @NonNull String[] permissions,
										   @NonNull int[] grantResults) {
        /*
         * if any permission failed, the sample could not play
         */
		if (PERMISSION_REQUEST_CODE_CAMERA != requestCode) {
			super.onRequestPermissionsResult(requestCode,
					permissions,
					grantResults);
			return;
		}

		Assert.assertEquals(grantResults.length, 1);
		cameraGranted_ = (grantResults[0] == PackageManager.PERMISSION_GRANTED);
		if (cameraGranted_) {
			CreateCameraPreviewEngine();
		}
	}

	private void CreateCameraPreviewEngine() {
		int rotation = 90 * ((WindowManager)(getSystemService(WINDOW_SERVICE)))
					.getDefaultDisplay()
					.getRotation();
		// create a new thread to create Camera
		nativeCamera_ = CreateCamera(width_, height_, rotation);
		cameraWidth_ = GetCameraCompatibleWidth();
		cameraHeight_ = GetCameraCompatibleHeight();

		// Now create TextureView
		textureView_ = new TextureView(this);
		textureView_.setSurfaceTextureListener(this);
		setContentView(textureView_);

	}

	private native void notifySurfaceTextureChanged(Surface surface);
	private native void notifySurfaceTextureCreated(Surface surface);
	private native void notifySurfaceTextureDestroyed(Surface surface);
	/*
	 * Create a camera mgr, select backfacing camera, and find the best resolution
	 * for display mode. The returned type is Native side object
	 */
	private native long CreateCamera(int width, int height, int rotation);
	private native int  GetCameraCompatibleWidth();
	private native int  GetCameraCompatibleHeight();
	private native boolean CreateSession(long cameraEngine);
	private native boolean StartPreview(long nativeCamera);

	static {
		System.loadLibrary("camera_view");
	}
}