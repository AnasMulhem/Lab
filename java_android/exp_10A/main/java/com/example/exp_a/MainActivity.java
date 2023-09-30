package com.example.exp_a;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends AppCompatActivity {

    private EditText editText;
    private Button share;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        share=(Button) findViewById(R.id.button);
        editText=(EditText) findViewById(R.id.editTextTextPersonName);
    }

    public void sharing(View view){
        Intent intent = new Intent();
        intent.putExtra(Intent.EXTRA_TEXT,editText.getText().toString());
        intent.setAction(Intent.ACTION_SEND);
        intent.setType("text/plain");

        Intent share_intent = Intent.createChooser(intent, null);
        startActivity(share_intent);
    }
}