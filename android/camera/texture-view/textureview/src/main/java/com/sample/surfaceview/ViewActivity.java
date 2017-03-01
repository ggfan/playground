package com.sample.surfaceview;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.SurfaceTexture;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;

import junit.framework.Assert;

public class ViewActivity extends Activity
		implements TextureView.SurfaceTextureListener,
		ActivityCompat.OnRequestPermissionsResultCallback {
	private  TextureView textureView_;
	private  int width_, height_;
	boolean  cameraGranted_ = false;
    boolean  startPending_ = false;
	Surface  surface_ = null;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
        Display display = getWindowManager().getDefaultDisplay();
		int height_ = display.getMode().getPhysicalHeight();
		int width_ = display.getMode().getPhysicalWidth();

		textureView_ = new TextureView(this);
		textureView_.setSurfaceTextureListener(this);
		setContentView(textureView_);
		RequestCamera();
	}

	public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        // ReaquestCamera(ANativeWindow);
		surface_ = new Surface(surface);
		if (cameraGranted_) {
			notifySurfaceTextureCreated(surface_);
		} else {
			startPending_ = true;
		}
	}

	public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
		// Ignored, Camera does all the work for us
		notifySurfaceTextureChanged(surface_);
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
		if (cameraGranted_ && startPending_) {
			// some race condition, to be improved
			notifySurfaceTextureCreated(surface_);
		}
	}

	private native void notifySurfaceTextureChanged(Surface surface);
	private native void notifySurfaceTextureCreated(Surface surface);
	private native void notifySurfaceTextureDestroyed(Surface surface);

	static {
		System.loadLibrary("camera_view");
	}
}