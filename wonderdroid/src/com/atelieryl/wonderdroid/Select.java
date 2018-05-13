
package com.atelieryl.wonderdroid;

import java.io.File;
import java.util.Random;

import com.actionbarsherlock.view.Menu;
import com.actionbarsherlock.view.MenuInflater;
import com.actionbarsherlock.view.MenuItem;
import com.google.ads.AdRequest;
import com.google.ads.AdSize;
import com.google.ads.AdView;

import com.atelieryl.wonderdroid.utils.RomAdapter;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;

import android.widget.Adapter;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.BaseAdapter;
import android.widget.FrameLayout;
import android.widget.Gallery;
import android.widget.GridView;
import android.widget.ImageView;
import android.widget.ListAdapter;
import android.widget.SpinnerAdapter;
import android.widget.TextView;
import android.widget.Toast;

@SuppressWarnings("deprecation")
public class Select extends BaseActivity {

    private static final String TAG = Select.class.getSimpleName();
    
    private static String currentRomPath = "";

    private final Runnable bgSwitcher = new Runnable() {

        private int splashindex = -1;

        private final Random mRNG = new Random();

        private Bitmap backgroundOne = null;

        @Override
        public void run() {

        	if (splashindex != -1) return;
        	
            int count = mRAdapter.getCount();
            if (count < 1/*0*/) { // Dont bother if we have less than 10 games
                return;
            }

            // Loop this shizzle
            //handler.removeCallbacks(this);
            //handler.postDelayed(this, 4000);
            //

            int attempt = 0;
            int newindex = mRNG.nextInt(count);
            Bitmap newbitmap = mRAdapter.getBitmap(newindex);
            
            while ((newindex == splashindex || newbitmap == null) && attempt < 10) {
            	newindex = mRNG.nextInt(count);
                newbitmap = mRAdapter.getBitmap(newindex);
                attempt++;
            }

            if (newindex == splashindex || newbitmap == null) {
                return;
            }

            splashindex = newindex;

            // first run
            if (backgroundOne == null) {
                backgroundOne = newbitmap;
                mBG1.setImageBitmap(backgroundOne);
                return;
            }

            mBG2.setImageBitmap(backgroundOne);
            mBG2.setVisibility(View.VISIBLE); // hide the new splash before
                                              // switching
            mBG1.setImageBitmap(newbitmap);
            backgroundOne = newbitmap;

            mBG2.startAnimation(fade);
        }
    };

    private Animation fade;

    private Handler handler;

    private AssetManager mAssetManager;

    private ImageView mBG1;

    private ImageView mBG2;

    private RomAdapter mRAdapter;

    private TextView mScreenFormat;

    private Gallery gallery;

    private GridView grid;

    private boolean adSupported = true;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.select);
        gallery = (Gallery)this.findViewById(R.id.select_gallery);
        if (gallery == null)
            grid = (GridView)this.findViewById(R.id.select_grid);

        handler = new Handler();
        fade = AnimationUtils.loadAnimation(this, R.anim.splashfade);
        fade.setAnimationListener(new AnimationListener() {

            @Override
            public void onAnimationEnd(Animation animation) {
                mBG2.setVisibility(View.GONE);
            }

            @Override
            public void onAnimationRepeat(Animation animation) {
            }

            @Override
            public void onAnimationStart(Animation animation) {
                mBG2.setVisibility(View.VISIBLE);
            }
        });

        findViewById(android.R.id.content).setBackgroundColor(Color.parseColor("#303030"));
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getSupportMenuInflater();
        inflater.inflate(R.menu.menu_select, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
    	AlertDialog.Builder builder;
        builder = new AlertDialog.Builder(this);
        // Handle item selection
        switch (item.getItemId()) {
            /*case R.id.select_exitmi:
                this.finish();
                return true;*/
            case R.id.select_prefsmi:
                Intent intent = new Intent(this, Prefs.class);
                startActivity(intent);
                return true;
            case R.id.select_reportmi:
                builder.setTitle(R.string.report)
                .setMessage(R.string.reportdescription)
                .setPositiveButton(R.string.issues, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) { 
                    	openURL("https://github.com/williehwc/wonderdroid/issues");
                    }
                 })
                .setNegativeButton(R.string.close, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) { 
                        // do nothing
                    }
                 })
                .show();
            	return true;
            case R.id.select_moreappsmi:
            	openURL("https://play.google.com/store/apps/dev?id=8429993243664540065");
            	return true;
            case R.id.select_aboutmi:
                builder.setTitle(R.string.about)
                .setMessage(R.string.aboutdescription)
                .setPositiveButton(R.string.visit, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) { 
                    	openURL("http://yearbooklabs.com/");
                    }
                 })
                .setNegativeButton(R.string.close, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) { 
                        // do nothing
                    }
                 })
                .show();
            	return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
    
    private void openURL(String url) {
    	try {
    		Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse(url));
    		startActivity(browserIntent);
    	} catch (Exception e) {
    		android.text.ClipboardManager clipboard = (android.text.ClipboardManager) getSystemService(Context.CLIPBOARD_SERVICE);
		    clipboard.setText(url);
    		AlertDialog.Builder builder = new AlertDialog.Builder(this);
    		builder.setTitle(R.string.cannotopenurl)
            .setMessage(R.string.cannotopenurldescription)
            .setPositiveButton(R.string.close, new DialogInterface.OnClickListener() {
            	public void onClick(DialogInterface dialog, int which) { 
                    // do nothing
                }
             })
            .show();
    	}
    }

    private void startEmu(int romid) {
        Intent intent = new Intent(this, Main.class);
        intent.putExtra(Main.ROM, mRAdapter.getItem(romid));
        intent.putExtra(Main.ROMHEADER, mRAdapter.getHeader(romid));
        startActivity(intent);
    }

    @Override
    protected void onPause() {
        super.onPause();
        handler.removeCallbacks(bgSwitcher);
    }

    AdView ad = null;

    private void parseSupportOptions() {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        adSupported = prefs.getBoolean("adsupported", false);
    }

    @Override
    protected void onResume() {
        super.onResume();

        File romdirx = getWonderDroidApplication().getRomDir();
        if (romdirx == null) {
            Toast.makeText(this, R.string.nosdcard, Toast.LENGTH_LONG).show();
            return;
        }

        String sdpath = romdirx.getAbsolutePath();

        parseSupportOptions();

        /*if (adSupported) {
            FrameLayout adbox = (FrameLayout)findViewById(R.id.adbox);
            ad = new AdView(this, AdSize.BANNER, "a14fbeecba23019");
            AdRequest r = new AdRequest();
            r.addTestDevice("6A3DABBD306114452F0D233CDADCF438");
            ad.loadAd(r);
            adbox.addView(ad);
        }*/

        //
        
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(this);
        
        String romPath = prefs.getString("emu_rompath", "wonderdroid");
        if (!romPath.startsWith("/")) {
        	romPath = "/" + romPath;
        }
        if (!romPath.endsWith("/")) {
        	romPath = romPath + "/";
        }
        
        String memPath = prefs.getString("emu_mempath", "wonderdroid/cartmem");
        if (!memPath.startsWith("/")) {
        	memPath = "/" + memPath;
        }
        if (!memPath.endsWith("/")) {
        	memPath = memPath + "/";
        }
        
        if (!currentRomPath.equals("") && !romPath.equals(currentRomPath)) {
        	Intent intent = getIntent();
        	finish();
        	startActivity(intent);
        }
        currentRomPath = romPath;
        
        File romdir = new File(sdpath + romPath);
        romdir.mkdirs();
        File cartmemdir = new File(sdpath + memPath);
        cartmemdir.mkdirs();
        //

        mScreenFormat = (TextView)this.findViewById(R.id.select_screenformat);
        mAssetManager = this.getAssets();
        mRAdapter = new RomAdapter(this.getBaseContext(), sdpath + romPath, mAssetManager);

        if (mRAdapter.getCount() != 0) {

            ((TextView)this.findViewById(R.id.select_noroms)).setVisibility(View.GONE);

            mScreenFormat.setVisibility(View.VISIBLE);

            setupGalleryOrGrid(mRAdapter, new OnItemClickListener() {

                @Override
                public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
                    startEmu(arg2);
                }

            }, new OnItemSelectedListener() {

                @Override
                public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2, long arg3) {

                    WonderSwanHeader header = mRAdapter.getHeader(arg2);
                    if (header != null) {
                        String newtext;
                        if (header.isColor) {
                            newtext = getString(R.string.colour);
                        } else {
                            newtext = getString(R.string.mono);
                        }

                        if (header.isVertical) {
                            newtext += getString(R.string.vertical);
                        } else {
                            newtext += getString(R.string.horizontal);
                        }

                        mScreenFormat.setText(newtext);
                    }
                }

                @Override
                public void onNothingSelected(AdapterView<?> arg0) {
                }

            });

            mBG1 = (ImageView)this.findViewById(R.id.select_bg1);
            mBG2 = (ImageView)this.findViewById(R.id.select_bg2);
            if (prefs.getBoolean("showbackground", true)) {
            	mBG1.setVisibility(View.VISIBLE);
            	bgSwitcher.run();
            } else {
            	mBG1.setVisibility(View.INVISIBLE);
            }
        } else {
        	((TextView)this.findViewById(R.id.select_noroms)).setText(getResources().getString(R.string.noroms).replace("???", romPath.substring(1)));
        }

    }

    private final void setupGalleryOrGrid(RomAdapter adapter,
            OnItemClickListener itemClickListener, OnItemSelectedListener itemSelectedListener) {
        if (gallery == null) {
            grid.setAdapter(mRAdapter);
            grid.setOnItemClickListener(itemClickListener);
            grid.setOnItemSelectedListener(itemSelectedListener);
        }

        else {
            gallery.setAdapter(mRAdapter);
            gallery.setOnItemClickListener(itemClickListener);
            gallery.setOnItemSelectedListener(itemSelectedListener);
        }
    }
}
