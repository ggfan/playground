package gfan.android.example.intents

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.widget.Toast
import gfan.android.example.intents.databinding.ActivityThirdBinding

class ThirdActivity : AppCompatActivity() {
    private lateinit var viewBinding:ActivityThirdBinding
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        viewBinding = ActivityThirdBinding.inflate(layoutInflater)
        setContentView(viewBinding.root)

        val parentKey = resources.getString(R.string.parent_name)
        val src = if (savedInstanceState != null) {
            savedInstanceState.getSerializable(parentKey) as String
        } else {
            intent.extras?.getString(parentKey)
        }

        viewBinding.textView2.text = "${viewBinding.textView2.text} $parentKey $src"
        Toast.makeText(applicationContext, "We are moved to second Activity", Toast.LENGTH_LONG).show()
    }
}