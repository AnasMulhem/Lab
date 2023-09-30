package com.example.exp_9;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.drawable.AnimationDrawable;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;

public class MainActivity extends AppCompatActivity {

    private Button start;
    private ImageView imageView;
    AnimationDrawable animationDrawable;
    private boolean animation=false;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        start=(Button) findViewById(R.id.button);
        imageView=(ImageView) findViewById(R.id.imageView);

        start.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(!animation){
                    start.setText(R.string.stop);
                    imageView.setBackgroundResource(R.drawable.animations);
                    animationDrawable = (AnimationDrawable) imageView.getBackground();
                    animation = true;
                    animationDrawable.start();
                }
                else {
                    start.setText(R.string.start);
                    animationDrawable.stop();
                    animation= false;
                }
            }
        });

    }
}