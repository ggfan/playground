package com.google.example.round_trip;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

class Parameters {
    int   int1_;
    long  long1_;
    int   int2_;
    long  long2_;
    int   int3_;
    long  long3_;
    float float1_;
    float float2_;
    float float3_;
    char  type_;

    Parameters(int int1, int int2, int int3,
               long long1, long long2, long long3,
               float f1, float f2, float f3,
               char type) {
        int1_ = int1;    int2_ = int2;   int3_ = int3;
        long1_ = long1;  long2_ = long2; long3_ = long3;
        float1_ = f1;    float2_ = f2;   float3_ = f3;
        type_ = type;
    }
}

public class MainActivity extends AppCompatActivity {
    final int ITERATIONS = 100000;
    Button bnStart;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        TextView tvMessage = (TextView) findViewById(R.id.sample_text);
        bnStart = findViewById(R.id.startButton);
        bnStart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                onButtonClick(view);
            }
        });
    }

    // Handles the button clicking event
    // in a new thread --
    //    disable Button
    //    do computation
    //    display result to UI
    //    enable the button again
    public void onButtonClick(View v) {
        bnStart.setEnabled(false);
        final TextView tv =  (TextView) findViewById(R.id.sample_text);
        new Thread(new Runnable() {
            public void run() {
                final String msg = compareCallTimes();
                      // compareFieldExpense();
                      // compareParameterPassing();
                      // compareCallTimes();
                runOnUiThread(new Runnable() {
                    @Override
                    public void run() {
                        tv.setText(msg);
                        bnStart.setEnabled(true);
                    }
                });
            }
        }).start();
    }
    private String compareCallTimes() {
        int counter = 0;
        // Example of a call to a native method and get the
        // round trip time.
        long jniIncTime  = 0;
        long startTime = System.nanoTime();
        for (int i = 0; i < ITERATIONS; i++) {
            counter = jniInc(counter);
        }
        jniIncTime += System.nanoTime() - startTime;
        jniIncTime /= ITERATIONS;

        counter = 0;
        long javaIncTime = 0;
        startTime = System.nanoTime();
        for (int i = 0; i < ITERATIONS; i++) {
            counter = javaInc(counter);
        }
        javaIncTime += System.nanoTime() - startTime;
        javaIncTime /= ITERATIONS;

        long nativeIncTime = getPureNativeIncTime(ITERATIONS);

        String msg = "jniExeTime: " + jniIncTime +
                "\njavaExeTime: " + javaIncTime +
                "\nnativeExeTime: " + nativeIncTime;
        return msg;
    }
    int javaInc (int val) {
        return (val + 1);
    }
    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native int jniInc(int val);
    public native long getPureNativeIncTime(int iterations);

    // create 10 parameters and passing them to jni in 2 ways, then compare the round-trip time
    private String compareParameterPassing() {

        Parameters jniData = new Parameters(1, 2, 3,
                100, 200, 300,
                10.5f, 20.5f, 30.5f, 'a'
                );

        long objTime = 0;
        long valTime = 0;
        long startTime = System.nanoTime();
        for (int i = 0; i < ITERATIONS; i++) {
            passByValue(jniData.int1_, jniData.int2_, jniData.int3_,
                    jniData.long1_, jniData.long2_, jniData.long3_,
                    jniData.float1_, jniData.float2_, jniData.float3_,
                    jniData.type_);
        }
        valTime += System.nanoTime() - startTime;
        valTime /= ITERATIONS;

        startTime = System.nanoTime();
        for (int i = 0; i < ITERATIONS; i++) {
            passByObject(jniData, false);
        }
        objTime += System.nanoTime() - startTime;

        objTime /= ITERATIONS;
        String msg = "passByObjectTime: " + objTime + "\npassByValueTime: " + valTime;

        return msg;
    }

    public native void passByObject(Parameters obj, boolean fieldCache);
    public native void passByValue(int intVal1, int intVal2, int intVal3,
                                   long longVal1, long longVal2, long longVal3,
                                   float floatVal1, float floatVal2, float floatVal3,
                                   char type);

    private String compareFieldExpense() {
        Parameters jniData = new Parameters(1, 2, 3,
                100, 200, 300,
                10.5f, 20.5f, 30.5f, 'a'
        );
        long getFieldTime = getFieldTime(jniData, ITERATIONS);

        return "getFieldExpense: " + getFieldTime;
    }

    public native long getFieldTime(Parameters jniData,long iterations);

}

/*
   Execution Time on Pixel XL ( in ns ):
     1) call time
         jniCall: 224
         javaVall: 1
         nativeCall: 0

     2) passing parameter list
         pass by objects:  6972
         pass by value:    260

         pass by objects + class/field caching: 1792
         pass by value:   216

     3) get classID and fieldID on:
         854ns
 */
