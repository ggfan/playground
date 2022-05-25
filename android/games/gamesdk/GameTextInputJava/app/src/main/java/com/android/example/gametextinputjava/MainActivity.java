package com.android.example.gametextinputjava;

import static android.view.inputmethod.EditorInfo.IME_ACTION_NONE;

import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;

import android.content.Context;
import android.os.Bundle;
import android.text.InputType;
import android.util.AttributeSet;
import android.view.View;
import android.view.inputmethod.EditorInfo;
import android.widget.TextView;

import com.android.example.gametextinputjava.databinding.ActivityMainBinding;
import com.google.androidgamesdk.gametextinput.GameTextInput;
import com.google.androidgamesdk.gametextinput.InputConnection;
import com.google.androidgamesdk.gametextinput.Listener;
import com.google.androidgamesdk.gametextinput.Settings;
import com.google.androidgamesdk.gametextinput.State;

public class MainActivity extends AppCompatActivity {
    private InputEnabledTextView inputEnabledTextView;
    // Used to load the 'GameTextInput' library on application startup.
    static {
        System.loadLibrary("GameTextInput");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        initNativeTextInput();

        inputEnabledTextView = binding.inputEnabledTextView;
        inputEnabledTextView.createInputConnection(InputType.TYPE_CLASS_TEXT);
        setInputConnectionNative(inputEnabledTextView.mInputConnection);

        binding.button.setOnClickListener(view->showIme());
    }

    public ActivityMainBinding getBinding() {
        return binding;
    }

    @Override
    protected void onDestroy() {
        terminateNativeTextInput();
        super.onDestroy();
    }

    /**
     * A native method that is implemented by the 'gametextinputjava' native library,
     * which is packaged with this application.
     */
    public native void showIme();

    public native boolean initNativeTextInput();
    public native boolean terminateNativeTextInput();

    private native void setInputConnectionNative(InputConnection connection);
}
