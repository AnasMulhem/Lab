package com.example.exp_1;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.Color;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;



public class MainActivity extends AppCompatActivity {

    private Button enter,clear;
    private RadioGroup radioGroup;
    private RadioButton Red,Green,Blue;
    private TextView textview;
    private EditText editText;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        enter=(Button)findViewById(R.id.enter_button);
        clear=(Button)findViewById(R.id.clear);

        editText=(EditText) findViewById(R.id.editTextTextPersonName);
        radioGroup=(RadioGroup) findViewById(R.id.radiogroup);
        Red=(RadioButton) findViewById(R.id.redbutton);
        Green=(RadioButton) findViewById(R.id.greenbutton);
        Blue=(RadioButton) findViewById(R.id.bluebutton);
        textview=(TextView) findViewById(R.id.textView);

        clear.setOnClickListener(new View.OnClickListener()
        {
            @Override
             public void onClick(View view)
            {
                textview.setTextColor(Color.BLACK);
                textview.setText("My color text");
            }

        });

        enter.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
               textview.setText(editText.getText());
            }
        });

        radioGroup.setOnCheckedChangeListener((radioGroup,i)->{
            if(i==R.id.redbutton){
                textview.setTextColor(Color.RED);
            } else if (i==R.id.greenbutton) {
                textview.setTextColor(Color.GREEN);
            } else if (i==R.id.bluebutton) {
                textview.setTextColor(Color.BLUE);
            }
        });


    }
}