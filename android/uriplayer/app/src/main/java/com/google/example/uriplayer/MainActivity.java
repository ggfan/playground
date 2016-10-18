package com.google.example.uriplayer;

import android.Manifest;
import android.content.pm.PackageManager;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    Button create;
    Button play;

    boolean playerCreated = false;
    boolean playerPlaying = false;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        create = (Button) findViewById(R.id.create);
        create.setOnClickListener( new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!playerCreated) {
                    playerCreated = (createURIPlayer("http://upload.wikimedia.org/wikipedia/commons/6/6d/Banana.ogg")
                     != 0);
                }
            }
        });

        play = (Button)findViewById(R.id.play);
        play.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (playerPlaying || !playerCreated)
                    return;
                playerPlaying = (startURIPlayer(true) != 0);
            }
        });

        assert(PackageManager.PERMISSION_GRANTED == ContextCompat.checkSelfPermission(this, Manifest.permission.INTERNET));
        assert(PackageManager.PERMISSION_GRANTED == ContextCompat.checkSelfPermission(this, Manifest.permission.MODIFY_AUDIO_SETTINGS));
    }

    @Override
    public  void onDestroy() {
        deleteURIPlayer();
        playerCreated = false;
        playerPlaying = false;
        super.onDestroy();
    }

    public native int createURIPlayer(String uri);
    public native int startURIPlayer(boolean play);
    public native void deleteURIPlayer();

}
