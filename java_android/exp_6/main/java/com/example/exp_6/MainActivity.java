package com.example.exp_6;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private Button test;
    private EditText editText;
    private TextView textView;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        test=(Button) findViewById(R.id.test);
        textView=(TextView) findViewById(R.id.textView);
        editText=(EditText) findViewById(R.id.editTextTextPersonName);

        test.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent intent= new Intent(MainActivity.this,MainActivity2.class);
                intent.putExtra("extra",editText.getText().toString());
                startActivity(intent);
            }
        });

    }
}