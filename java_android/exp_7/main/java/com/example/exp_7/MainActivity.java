package com.example.exp_7;

import androidx.appcompat.app.AppCompatActivity;

import android.media.MediaMetadataRetriever;
import android.media.MediaPlayer;
import android.os.Bundle;
import android.os.StrictMode;
import android.view.View;
import android.webkit.URLUtil;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import java.io.IOException;
import java.util.HashMap;

public class MainActivity extends AppCompatActivity {

    Button play;

    private TextView title,artist;

    private EditText enterurl;

    String current_url;
    String prev="";
    private MediaPlayer mediaPlayer;



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        play=(Button) findViewById(R.id.play);
        title=(TextView) findViewById(R.id.textView);
        artist=(TextView) findViewById(R.id.textView2);
        enterurl=(EditText) findViewById(R.id.editTextTextPersonName);

        mediaPlayer=new MediaPlayer();

        play.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View view)
            {
                current_url=enterurl.getText().toString();
                if(mediaPlayer!=null)
                {
                    if(mediaPlayer.isPlaying()){
                        mediaPlayer.pause();
                        play.setText("PLAY");
                    }
                    else
                    {
                        if(URLUtil.isHttpsUrl(current_url)){
                            if(!prev.equals(current_url))
                            {
                                mediaPlayer.reset();
                                try {
                                    mediaPlayer.setDataSource(enterurl.getText().toString());
                                    mediaPlayer.prepare();
                                } catch (IOException e) {
                                    throw new RuntimeException(e);
                                }
                                prev=current_url;
                            }
                            MediaMetadataRetriever retriever=new MediaMetadataRetriever();
                            retriever.setDataSource(current_url,new HashMap<String,String>());
                            String title1 = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_TITLE);
                            String artist1 = retriever.extractMetadata(MediaMetadataRetriever.METADATA_KEY_ARTIST);
                            artist.setText("Artist" + " : " + artist1);
                            title.setText("Title" + " : " + title1);
                            play.setText("PAUSE");
                            mediaPlayer.start();
                        }
                        else
                        {
                            Toast toast = Toast.makeText(getBaseContext(),"Invalid URL ! ",Toast.LENGTH_LONG);
                            toast.show();
                        }
                    }
                }
            }
        });


    }
}