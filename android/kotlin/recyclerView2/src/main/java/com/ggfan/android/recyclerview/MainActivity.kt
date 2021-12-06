package com.ggfan.android.recyclerview

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.TextView
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.example.android.camera.utils.GenericListAdapter

class MainActivity : AppCompatActivity() {
    private val qualityStrings = listOf("Customized","UHD", "HD", "SD")
    private lateinit var recyclerView:RecyclerView
    private var qualitySelectorIndex = 2

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        recyclerView = findViewById(R.id.qualitySelector)
        initializeQualitySectionsUI()
    }

    private fun initializeQualitySectionsUI() {
        val selectorStrings = qualityStrings

        // create the adapter to QualitySelector RecyclerView
        recyclerView.apply {
            layoutManager = LinearLayoutManager(context)
            adapter = GenericListAdapter(
                selectorStrings,
                itemLayoutId = R.layout.item_text_view
            ) { holderView, qcString, position ->

                holderView.findViewById<TextView>(R.id.textView)?.apply {
                    text = qcString
                    isSelected = (position == qualitySelectorIndex)
                }
                holderView.setOnClickListener {
                    if (qualitySelectorIndex == position) return@setOnClickListener

                    recyclerView.findViewHolderForAdapterPosition(qualitySelectorIndex)
                        ?.itemView
                        ?.isSelected = false
                    recyclerView.adapter?.notifyItemChanged(qualitySelectorIndex)

                    it.isSelected = true
                    qualitySelectorIndex = position
                    recyclerView.adapter?.notifyItemChanged(qualitySelectorIndex)
                }
            }
        }
    }
}

private val TAG="RecyclerView"