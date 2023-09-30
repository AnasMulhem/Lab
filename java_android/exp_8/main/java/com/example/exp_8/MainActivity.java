package com.example.exp_8;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private TextView Upper,Down;
    private Spinner spinner;
    private Button select;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        select=(Button) findViewById(R.id.button);
        Upper=(TextView) findViewById(R.id.upper);
        Down=(TextView) findViewById(R.id.down);
        spinner=(Spinner) findViewById(R.id.spinner);

        String[] contacts_array = getResources().getStringArray(R.array.Contacts);
        ArrayAdapter<String> adapter = new ArrayAdapter<>(this,
                androidx.appcompat.R.layout.support_simple_spinner_dropdown_item,contacts_array);
        spinner.setAdapter(adapter);

        select.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                String selected = spinner.getSelectedItem().toString();
                String new_string = "Selected Name :: "+ selected;
                Down.setText(new_string);
            }
        });
    }
}