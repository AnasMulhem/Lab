package com.example.exp_5;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private Button calculate;
    private TextView textview_upp;
    private TextView textview_down;
    private EditText editText;

    static final int one=1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        calculate=(Button) findViewById(R.id.calc);
        textview_upp=(TextView) findViewById(R.id.textView2);
        textview_down=(TextView) findViewById(R.id.result1);
        editText=(EditText) findViewById(R.id.editTextTextPersonName);

        final Intent intent=new Intent(this,MainActivity2.class);

        calculate.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                String str=editText.getText().toString();
                intent.putExtra("extra",str);
                startActivityForResult(intent,one);
            }
        });
    }
    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        double result;
        String result_string;

        if ( requestCode == one ){
            if( resultCode == RESULT_OK ){
                result = data.getDoubleExtra("extra", 0);
                result_string = getString(R.string.result) + Double.valueOf(result);
                textview_down.setText(result_string);
            }
        }
    }
}