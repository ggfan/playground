/**
 To proof code snippet for CameraX quality section.
 */
package com.ggfan.android.video_quality_listview

import androidx.appcompat.app.AppCompatActivity
import android.annotation.SuppressLint
import android.hardware.camera2.CameraCharacteristics
import android.hardware.camera2.CameraMetadata
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.view.View
import android.view.WindowInsets
import android.widget.ArrayAdapter
import androidx.camera.camera2.interop.Camera2CameraInfo
import androidx.camera.lifecycle.ProcessCameraProvider
import androidx.camera.video.Quality
import androidx.camera.video.QualitySelector
import androidx.camera.video.Recorder
import com.ggfan.android.video_quality_listview.databinding.ActivityListViewBinding
import java.lang.IllegalArgumentException

/**
 * An example full-screen activity that shows and hides the system UI (i.e.
 * status bar and navigation/system bar) with user interaction.
 */
class ListViewActivity : AppCompatActivity() {

    private lateinit var viewBinding: ActivityListViewBinding
    private val hideHandler = Handler()

    @SuppressLint("InlinedApi")
    private val hidePart2Runnable = Runnable {
        // Delayed removal of status and navigation bar
        if (Build.VERSION.SDK_INT >= 30) {
            viewBinding.simpleQualityListView.windowInsetsController?.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
        } else {
            // Note that some of these constants are new as of API 16 (Jelly Bean)
            // and API 19 (KitKat). It is safe to use them, as they are inlined
            // at compile-time and do nothing on earlier devices.
            viewBinding.simpleQualityListView.systemUiVisibility =
                View.SYSTEM_UI_FLAG_LOW_PROFILE or
                    View.SYSTEM_UI_FLAG_FULLSCREEN or
                    View.SYSTEM_UI_FLAG_LAYOUT_STABLE or
                    View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or
                    View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or
                    View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
        }
    }
    private val showPart2Runnable = Runnable {
        // Delayed display of UI elements
        supportActionBar?.show()
    }
    private var isFullscreen: Boolean = false


    @SuppressLint("ClickableViewAccessibility")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        viewBinding = ActivityListViewBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)

        supportActionBar?.setDisplayHomeAsUpEnabled(true)

        isFullscreen = true

        val cameraProvider = ProcessCameraProvider.getInstance(this).get()

        // snippet code
        val cameraInfo = cameraProvider.availableCameraInfos.filter {
            Camera2CameraInfo
                .from(it)
                .getCameraCharacteristic(CameraCharacteristics.LENS_FACING) == CameraMetadata.LENS_FACING_BACK
        }

        val supportedQualities = QualitySelector.getSupportedQualities(cameraInfo[0])
        val filteredQualities = arrayListOf (Quality.UHD, Quality.FHD, Quality.HD, Quality.SD)
            .filter { supportedQualities.contains(it) }

        // Use a simple ListView with the id of simple_quality_list_view
        viewBinding.simpleQualityListView.apply {
            adapter = ArrayAdapter(context,
                android.R.layout.simple_list_item_1,
                filteredQualities.map { it.qualityToString() })

            // Set up the user interaction to manually show or hide the system UI.
            setOnItemClickListener { _, _, position, _ ->
                // Inside View.OnClickListener,
                // convert Quality.* constant to QualitySelector
                val qualitySelector = QualitySelector.from(filteredQualities[position])

                // Create a new Recorder/VideoCapture for the new quality
                // and bind to lifecycle
                val recorder = Recorder.Builder()
                    .setQualitySelector(qualitySelector).build()

                // ...
            }
        }
    }

    private fun show() {
        // Show the system bar
        viewBinding.simpleQualityListView.apply {
            if (Build.VERSION.SDK_INT >= 30) {
                windowInsetsController?.show(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
            } else {
                this.systemUiVisibility =
                    View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
                        View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
            }
            isFullscreen = true

            // Schedule a runnable to display UI elements after a delay
            hideHandler.removeCallbacks(hidePart2Runnable)
            hideHandler.postDelayed(showPart2Runnable, UI_ANIMATION_DELAY.toLong())
        }
    }

    companion object {
        /**
         * Whether or not the system UI should be auto-hidden after
         * [AUTO_HIDE_DELAY_MILLIS] milliseconds.
         */
        private const val AUTO_HIDE = true

        /**
         * If [AUTO_HIDE] is set, the number of milliseconds to wait after
         * user interaction before hiding the system UI.
         */
        private const val AUTO_HIDE_DELAY_MILLIS = 3000

        /**
         * Some older devices needs a small delay between UI widget updates
         * and a change of the status and navigation bar.
         */
        private const val UI_ANIMATION_DELAY = 300
    }
}

fun Quality.stringToQuality(name: String) : Quality {
    return when (name) {
        "UHD" -> Quality.UHD
        "FHD" -> Quality.FHD
        "HD" -> Quality.HD
        "SD" -> Quality.SD
        else -> throw IllegalArgumentException()
    }
}

fun Quality.qualityToString() : String {
    return when (this) {
        Quality.UHD -> "UHD"
        Quality.FHD -> "FHD"
        Quality.HD -> "HD"
        Quality.SD -> "SD"
        else -> throw IllegalArgumentException()
    }
}