package com.example.game_activity

import android.os.Bundle
import android.view.View
import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    override fun onResume() {
        super.onResume()
        val decorView = window.decorView
        decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or
                View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or
                View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
                View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
                View.SYSTEM_UI_FLAG_FULLSCREEN
    }

    companion object {
        init {
            System.loadLibrary("GameActivity")
        }
    }
}