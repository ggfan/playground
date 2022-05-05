package com.android.example.two_libs

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.TextView
import com.android.example.two_libs.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // Example of a call to a native method
        binding.sampleText.text = nativeInit()
    }

    fun textOnClick(view: View) {
        nativeHandleOnClick();
    }

    override fun onDestroy() {
        nativeClose()
        super.onDestroy()
    }

    /**
     * A native method that is implemented by the 'two_libs' native library,
     * which is packaged with this application.
     */
    external fun nativeInit(): String
    external fun nativeHandleOnClick(): Boolean
    external fun nativeClose()

    companion object {
        // Used to load the 'two_libs' library on application startup.
        init {
            System.loadLibrary("two_libs")
        }
    }
}