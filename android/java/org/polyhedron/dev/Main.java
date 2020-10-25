package org.polyhedron.dev;

import android.content.Context;
import android.content.res.AssetManager;
import android.os.Bundle;

import org.libsdl.app.SDLActivity;
import java.lang.ref.WeakReference;

public class Main extends SDLActivity
{
    static final String TAG = "POLYHDRN";
    public static WeakReference<Main> mainActivity = null;
    private AssetManager m_AssetManager = null;

//    public static native void nativeOnCreate(AssetManager assetManager, Context context);

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        mainActivity = new WeakReference<Main>(this);

		Thread.currentThread().setName("Main/UI");
        m_AssetManager = getAssets();

//        nativeOnCreate(m_AssetManager, this.getBaseContext());
    }

    @Override
    protected String getMainSharedObject()
    {
        return getContext().getApplicationInfo().nativeLibraryDir + "/libPolyhedron.so";
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "hidapi",
        };
    }

    static {
        System.loadLibrary("Polyhedron");
    }
}
