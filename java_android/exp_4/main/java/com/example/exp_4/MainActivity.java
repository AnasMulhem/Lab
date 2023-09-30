package com.example.exp_4;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.StrictMode;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import java.io.IOException;
import java.io.InputStream;
import java.net.HttpURLConnection;
import java.net.URL;

public class MainActivity extends AppCompatActivity {

    private ImageView image;
    private String str1="https://softlab.technion.ac.il/exp/android/meet_01/task/files/on.png";
    private String str2="https://softlab.technion.ac.il/exp/android/meet_01/task/files/off.png";
    private TextView textView;
    private boolean check=false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
        StrictMode.setThreadPolicy(policy);
        image=(ImageView) findViewById(R.id.imageView);
        textView=(TextView) findViewById(R.id.textView);


        try {
            image.setImageBitmap(drawableFromUrl(str1));
        } catch (IOException e) {
            throw new RuntimeException(e);
        }

        image.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(!check)
                {
                    try {
                        image.setImageBitmap(drawableFromUrl(str2));
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    check=true;
                }
                else {
                    try {
                        image.setImageBitmap(drawableFromUrl(str1));
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    check=false;
                }
            }
        });
    }
    Bitmap drawableFromUrl(String url) throws java.net.MalformedURLException,java.io.IOException {
        Bitmap x;
        HttpURLConnection connection = (HttpURLConnection)new URL(url) .openConnection();
        connection.setRequestProperty("User-agent","Mozilla/4.0");
        try {
            connection.connect();
        } catch (IOException e) {
            e.printStackTrace();
        }
        InputStream input = null;
        try {
            input = connection.getInputStream();
        } catch (IOException e) {
            e.printStackTrace();
        }
        x = BitmapFactory.decodeStream(input);
        return x;
    }
}