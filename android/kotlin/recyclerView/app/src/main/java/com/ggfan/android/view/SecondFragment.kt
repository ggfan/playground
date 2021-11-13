package com.ggfan.android.view

import android.os.Bundle
import android.util.Log
import androidx.fragment.app.Fragment
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.core.content.ContextCompat
import androidx.navigation.fragment.findNavController
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.ggfan.android.view.databinding.FragmentSecondBinding

/**
 * A simple [Fragment] subclass as the second destination in the navigation.
 */
class SecondFragment : Fragment(), CustomAdapter.OnItemClickListener {

    private val  defaultTestData = arrayOf("one", "two", "three", "four", "five", "six", "seven")
    private var _binding: FragmentSecondBinding? = null
    private var prevView:View? = null
    private var curSelection = 0
    private var incrementValue = 1

    // This property is only valid between onCreateView and
    // onDestroyView.
    private val binding get() = _binding!!
    private var  index = 0

    override fun onCreateView(
        inflater: LayoutInflater, container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentSecondBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        binding.buttonSecond.setOnClickListener {
            findNavController().navigate(R.id.action_SecondFragment_to_FirstFragment)
        }

        // Set a selection to the recyclerView
        binding.buttonSelection.setOnClickListener {
            binding.recyclerView.apply {

                // this will scroll to the position but will not highlight it.
                // it is not a reliable service, but it is UI so error is ok.
                //   UI quality is not good.
                Log.d("TEST", "scrolling to index(start): $index, curSelction=$curSelection")
                // ScrollView could not scroll to the one that is not in the visible
                (layoutManager as LinearLayoutManager).scrollToPosition(index)
                val holder = findViewHolderForAdapterPosition(index)

                if (holder != null) {
                    holder.itemView.performClick()
                    /*
                    // this will dynamically swapping out the list.
                    adapter = if (index++ % 2 == 0) {
                        CustomAdapter(arrayOf("2", "4", "6", "8", "10"), this@SecondFragment)
                    } else {
                        CustomAdapter(arrayOf("1", "3", "5"), this@SecondFragment)
                    }
                    */
                    index += incrementValue
                    if (index >= defaultTestData.size || index < 0) {
                        incrementValue = -incrementValue
                        index += incrementValue * 2
                    }
                }
                Log.d("TEST", "scrolling to index(done): $index, curSelction=$curSelection")
            }
        }

        binding.recyclerView.apply {
            adapter = CustomAdapter(defaultTestData, this@SecondFragment)
            layoutManager = LinearLayoutManager(context)

            //*** the following code is useless: as it is ever get called. plus, the recyclerView
            //*** itself does not get hold anything.
            setOnClickListener{
                Log.i("Test", "==== RecyclerView clicked")
            }
        }.postDelayed({
            // binding.recyclerView.findViewHolderForAdapterPosition(curSelection)!!.itemView.performClick();
            prevView = binding.recyclerView.findViewHolderForAdapterPosition(curSelection)?.itemView
            prevView?.setBackgroundColor(ContextCompat.getColor(requireContext(), R.color.purple_200))
        }, 100)

    }
    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }

    override fun onItemClick(view: View?, position: Int) {
        if (view == null || curSelection == position)  return
        prevView?.setBackgroundColor(0x00)

        prevView = view
        curSelection = position

        prevView?.setBackgroundColor(ContextCompat.getColor(requireContext(), R.color.purple_200))
        Log.d("Test", "====Inside the onItemClickCallabck: $position")
    }
}

class CustomAdapter(private val dataSet: Array<String>, private val onItemClickListener: OnItemClickListener) :
    RecyclerView.Adapter<CustomAdapter.ViewHolder>() {

    /**
     * Provide a reference to the type of views that you are using
     * (custom ViewHolder).
     * https://stackoverflow.com/questions/27194044/how-to-properly-highlight-selected-item-on-recyclerview
     */
    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        private val textView: TextView = view.findViewById(R.id.textView)
        fun bindData(string:String) {
            textView.text = string
        }
    }
    // Create new views (invoked by the layout manager)
    override fun onCreateViewHolder(viewGroup: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(viewGroup.context)
            .inflate(R.layout.text_row_item, viewGroup, false)
        return ViewHolder(view)
    }

    // Replace the contents of a view (invoked by the layout manager)
    // method 1:  https://stackoverflow.com/questions/27194044/how-to-properly-highlight-selected-item-on-recyclerview
    // method 2:
    override fun onBindViewHolder(viewHolder: ViewHolder, position: Int) {
        // Get element from your dataset at this position and replace the
        // contents of the view with that element
        viewHolder.bindData(dataSet[position])

        // pass to the client's callback function.
        viewHolder.itemView.setOnClickListener {
            onItemClickListener.onItemClick(it, viewHolder.adapterPosition)
        }
    }

    // Return the size of your dataset (invoked by the layout manager)
    override fun getItemCount() = dataSet.size

    interface OnItemClickListener {
        fun onItemClick(view: View?, position: Int)
    }
}


