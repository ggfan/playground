package com.android.example.gameactivity.template;

import android.os.Bundle;
import com.google.androidgamesdk.GameActivity;

public class MainActivity extends GameActivity {

    // Used to load the 'template' library on application startup.
    static {
        System.loadLibrary("template");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }
}