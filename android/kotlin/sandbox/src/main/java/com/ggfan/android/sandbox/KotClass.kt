package com.ggfan.android.sandbox

import android.util.Log

/**
 * Sandbox for set/get() and secondary constructors
 */
class User constructor(_name:String) {
    val name:String
    init {
        Log.i("====", "in Init Block")
        name = _name
    }

    constructor(_name:String, _addr:String):this(_name) {
        address = _addr
        career = "nothing"
    }

    constructor(_name:String, _addr:String, _career:String): this(_name, _addr) {
        career = _career
    }
    var address: String = "Unspecified"
        set(value: String) {
            Log.i(
                "====", """
                Address was changed for $name:
                "$field" -> "$value".
            """.trimIndent()
            )
            field = value
        }
        get() = field

    var career: String = "unknown"
        private set(value: String) {
            Log.i("====", "setting career: $value")
            field = value
        }
        get() = field
}
