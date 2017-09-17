package com.imooc.extractinfotest;

import android.app.Activity;
import android.os.Environment;
import android.os.Bundle;
import android.view.Window;

public class MainActivity extends Activity
{
    static
    {
        System.loadLibrary("avutil-55");
        System.loadLibrary("swresample-2");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avformat-57");
        System.loadLibrary("swscale-4");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("nativextract");
    }

    public native int mp4ToAac(String srcPath , String audioPath);

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.activity_main);

        String path = Environment.getExternalStorageDirectory().getAbsolutePath() + "/sintel.mp4";
        int result = mp4ToAac(path , Environment.getExternalStorageDirectory().getAbsolutePath() + "/extra.aac");
    }

}
