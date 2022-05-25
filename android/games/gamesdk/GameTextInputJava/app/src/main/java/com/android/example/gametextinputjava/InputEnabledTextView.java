package com.android.example.gametextinputjava;

import static android.view.inputmethod.EditorInfo.IME_ACTION_NONE;
import static android.view.inputmethod.EditorInfo.IME_FLAG_NO_FULLSCREEN;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.inputmethod.EditorInfo;

import androidx.core.graphics.Insets;

import com.google.androidgamesdk.gametextinput.GameTextInput;
import com.google.androidgamesdk.gametextinput.InputConnection;
import com.google.androidgamesdk.gametextinput.Listener;
import com.google.androidgamesdk.gametextinput.Settings;
import com.google.androidgamesdk.gametextinput.State;

public class InputEnabledTextView extends View implements Listener {
    public InputConnection mInputConnection;
    public InputEnabledTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public InputEnabledTextView(Context context) {
        super(context);
    }
    public void createInputConnection(int inputType) {
        EditorInfo editorInfo = new EditorInfo();
        editorInfo.inputType = inputType;
        editorInfo.actionId = IME_ACTION_NONE;
        editorInfo.imeOptions = IME_FLAG_NO_FULLSCREEN;
        mInputConnection = new InputConnection(this.getContext(), this,
                new Settings(editorInfo, true)
        ).setListener(this);
    }

    @Override
    public InputConnection onCreateInputConnection(EditorInfo outAttrs) {
        if (outAttrs != null) {
            GameTextInput.copyEditorInfo(mInputConnection.getEditorInfo(), outAttrs);
        }
        return mInputConnection;
    }

    // Called when the IME input changes.
    @Override
    public void stateChanged(State newState, boolean dismissed) {
        onTextInputEventNative(newState);
    }
    @Override
    public void onImeInsetsChanged(Insets insets) {

    }

    private native void onTextInputEventNative(State softKeyboardEvent);
}
