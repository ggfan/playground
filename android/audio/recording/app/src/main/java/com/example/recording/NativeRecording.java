/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.recording;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class NativeRecording extends Activity
        implements ActivityCompat.OnRequestPermissionsResultCallback {

    final  int AUDIO_ECHO_REQUEST = 1;
    int sampleRate = 0;
    int bufSize = 0;

    /** Called when the activity is first created. */
    @Override
    @TargetApi(17)
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main);

        // initialize native audio system
        createEngine();

        /*
         * retrieve fast audio path sample rate and buf size; if we have it, we pass to native
         * side to create a player with fast audio enabled [ fast audio == low latency audio ];
         * IF we do not have a fast audio path, we pass 0 for sampleRate, which will force native
         * side to pick up the 8Khz sample rate.
         */
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            AudioManager myAudioMgr = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
            String nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE);
            sampleRate = Integer.parseInt(nativeParam);
            nativeParam = myAudioMgr.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER);
            bufSize = Integer.parseInt(nativeParam);
        }

        ((Button) findViewById(R.id.record)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                int status = ActivityCompat.checkSelfPermission(NativeRecording.this,
                        Manifest.permission.RECORD_AUDIO);
                if (status != PackageManager.PERMISSION_GRANTED) {
                    ActivityCompat.requestPermissions(
                            NativeRecording.this,
                            new String[]{Manifest.permission.RECORD_AUDIO},
                            AUDIO_ECHO_REQUEST);
                    return;
                }
                recordAudio();
            }
        });

        ((Button) findViewById(R.id.stop)).setOnClickListener(new OnClickListener() {
            public void onClick(View view) {
                stopRecording();
            }
        });

    }

    // Single out recording for run-permission needs
    static boolean created = false;
    private void recordAudio() {
        if (!created) {
            created = createAudioRecorder(sampleRate, bufSize);
        }
        if (created) {
            startRecording();
        }
    }

   /** Called when the activity is about to be destroyed. */
    @Override
    protected void onPause()
    {
        // should also stop recording if recording is in progress
        super.onPause();
    }

    /** Called when the activity is about to be destroyed. */
    @Override
    protected void onDestroy()
    {
        shutdown();
        super.onDestroy();
    }
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        /*
         * if any permission failed, the sample could not play
         */
        if (AUDIO_ECHO_REQUEST != requestCode) {
            super.onRequestPermissionsResult(requestCode, permissions, grantResults);
            return;
        }

        if (grantResults.length != 1  ||
                grantResults[0] != PackageManager.PERMISSION_GRANTED) {
            /*
             * When user denied the permission, throw a Toast to prompt that RECORD_AUDIO
             * is necessary; on UI, we display the current status as permission was denied so
             * user know what is going on.
             * This application go back to the original state: it behaves as if the button
             * was not clicked. The assumption is that user will re-click the "start" button
             * (to retry), or shutdown the app in normal way.
             */
            Toast.makeText(getApplicationContext(),
                    getString(R.string.NeedRecordAudioPermission),
                    Toast.LENGTH_SHORT)
                    .show();
            return;
        }

        // The callback runs on app's thread, so we are safe to resume the action
        recordAudio();
    }

    /** Native methods, implemented in jni folder */
    public static native void createEngine();
    public static native boolean createAudioRecorder(int rate, int samplesPerBuf);
    public static native void startRecording();
    public static native void stopRecording();
    public static native void shutdown();

    /** Load jni .so on initialization */
    static {
         System.loadLibrary("native-audio-jni");
    }

}
