package com.sample.surfaceview;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import junit.framework.Assert;

public class ViewActivity extends Activity
		implements SurfaceHolder.Callback,
		ActivityCompat.OnRequestPermissionsResultCallback {

	SurfaceHolder surfaceHolder_;
	boolean  cameraGranted_ = false;
    boolean startPending_ = false;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		setContentView(R.layout.activity_main);

		// Get SurfaceView, then its holder
		// install surface callback to hook to our callbacks
		SurfaceView surfaceView = (SurfaceView) findViewById(R.id.surfaceView);
		surfaceView.getHolder().addCallback(this);

		RequestCamera();
	}

	public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
		// Now that the size is known, set up the camera parameters and begin
		// the preview.
		if (cameraGranted_)
			notifyDisplaySurfaceChanged(holder.getSurface());
	}

	public void surfaceCreated(SurfaceHolder holder) {
		surfaceHolder_ = holder;
		if (cameraGranted_)
			notifyDisplaySurfaceCreated(holder.getSurface());
		else {
			startPending_ = true;
		}
	}

	public void surfaceDestroyed(SurfaceHolder holder) {
		// stop preview and release camera
		if(cameraGranted_)
			notifyDisplaySurfaceDestroyed(holder.getSurface());
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
			notifyDisplaySurfaceCreated(surfaceHolder_.getSurface());
		}
	}

	private native void notifyDisplaySurfaceChanged(Surface surface);
	private native void notifyDisplaySurfaceCreated(Surface surface);
	private native void notifyDisplaySurfaceDestroyed(Surface surface);

	static {
		System.loadLibrary("camera_view");
	}
}