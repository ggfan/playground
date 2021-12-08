package com.gfan.android.camerax_lib

import androidx.camera.extensions.ExtensionMode
import java.io.Externalizable
import java.lang.Exception
import java.lang.IllegalArgumentException

fun ExtensionMode_getName(mode:Int) :String = when (mode) {
    ExtensionMode.AUTO -> "AUTO"
    ExtensionMode.BOKEH -> "BOKEH"
    ExtensionMode.NIGHT -> "NIGHT"
    ExtensionMode.HDR -> "HDR"
    ExtensionMode.FACE_RETOUCH -> "FACE_RETOUCH"
    ExtensionMode.NONE -> "NONE"
    else -> throw Exception(IllegalArgumentException())
}

fun ExtensionMode_getMode(name:String):Int = when(name) {
    "AUTO" -> ExtensionMode.AUTO
    "BOKEH" -> ExtensionMode.BOKEH
    "NIGHT" -> ExtensionMode.NIGHT
    "HDR" -> ExtensionMode.HDR
    "FACE_RETOUCH" -> ExtensionMode.FACE_RETOUCH
    "NONE" -> ExtensionMode.NONE
    else -> throw Exception(IllegalArgumentException())
}

fun ExtensionMode_getSupportedModes():List<Int> {
    return listOf(
        ExtensionMode.NONE,
        ExtensionMode.AUTO,
        ExtensionMode.BOKEH,
        ExtensionMode.FACE_RETOUCH,
        ExtensionMode.HDR,
        ExtensionMode.NIGHT
    )
}