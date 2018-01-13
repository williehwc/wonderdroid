
package com.atelieryl.wonderdroid;

import com.actionbarsherlock.app.SherlockActivity;

import android.os.Bundle;
import android.view.WindowManager;

public class BaseActivity extends SherlockActivity {

    WonderDroid getWonderDroidApplication() {
        return (WonderDroid)getApplication();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        try {
        	getActionBar().setIcon(android.R.color.transparent);
        } catch (Exception e) {
        	
        }
    }

}
