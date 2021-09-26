/**
 * Intent demo:
 *   - Activity
 *   - layout
 *   - manifest
 *
 *   - Intent to connect Activities
 *
 */

package gfan.android.example.intents

import android.content.Intent
import android.net.Uri
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import gfan.android.example.intents.databinding.ActivityMainBinding

class MainActivity : AppCompatActivity() {
    private lateinit var viewBinding:ActivityMainBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewBinding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)
    }

    fun onExplicitIntentClick(@Suppress("UNUSED_PARAMETER") v : View) {
        Log.println(Log.INFO, "INTENT", "Received request for explicit intent")
        /*
        // start individual activities separately: android starts it in the reverse order.
        startActivity(Intent(baseContext, SecondActivity::class.java).putExtra("initiator", "MainActivity"))
        startActivity(Intent(baseContext, ThirdActivity::class.java).putExtra("initiator", "MainActivity"))
        */

        // start the 2 activities.
        startActivities(arrayOf(
            Intent(baseContext, ThirdActivity::class.java).putExtra("initiator", "MainActivity"),
            Intent(baseContext, SecondActivity::class.java).putExtra("initiator", "MainActivity"),
        ))
    }

    fun onImplicitIntentClick(@Suppress("UNUSED_PARAMETER") v: View) {
        Log.println(Log.INFO, "INTENT", "Received request for implicit intent")

        //    val intent = Intent(Intent.ACTION_VIEW)
        //    intent.data = Uri.parse("https://github.com/ggfan/playground/tree/master/android")
        //        intent.putExtra("initiator", "MainActivity")
        //    startActivity(intent)
        startActivity(Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/ggfan/playground/tree/master/android")))
    }
}