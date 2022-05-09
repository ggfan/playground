package com.example.gameactivity

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.google.androidgamesdk.GameActivity

class MainActivity : GameActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        startEngine()
    }

    companion object {
      init {
         System.loadLibrary("gameactivity-internal")
         System.loadLibrary("gameactivity-app")
      }
    }
    private external fun startEngine():Int
}