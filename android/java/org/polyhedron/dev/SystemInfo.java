package org.polyhedron.dev;

import android.app.ActivityManager;
import android.content.ComponentCallbacks2;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkInfo;
import android.os.Environment;
import android.os.StatFs;
import android.provider.Settings;
import android.support.v4.content.PermissionChecker;
import android.util.Log;


import java.io.File;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Locale;

import static android.content.Context.CONNECTIVITY_SERVICE;
import static org.polyhedron.dev.Main.TAG;

public class SystemInfo implements ComponentCallbacks2 {

    protected static ActivityManager.MemoryInfo m_Info = null;

    public SystemInfo()
    {
        if (m_Info == null)
        {
            m_Info = new ActivityManager.MemoryInfo();
        }
        if (Main.mainActivity.get() != null)
        {
            ActivityManager activityManager = (ActivityManager) Main.mainActivity.get().getSystemService(Context.ACTIVITY_SERVICE);
            if (activityManager != null)
            {
                activityManager.getMemoryInfo(m_Info);
            }
            else
            {
                Log.e(TAG, "Unable to get ActivityManager!");
            }
        }
        else
        {
            Log.e(TAG, "Unable to get MainActivity!");
        }
    }

    public static long getAvailableStorageSpace()
    {
		StatFs stat = new StatFs(getExternalFilesDir());
		long availableStorageSpace = stat.getAvailableBlocksLong() * stat.getBlockSizeLong();
		return availableStorageSpace;
    }

    public static long getAvailableMemory()
    {
    	return m_Info.availMem;
    }

    public static long getTotalMemory()
	{
		return m_Info.totalMem;
	}

    public static long getMemoryThreshold()
    {
    	return m_Info.threshold;
    }

    public static boolean isLowOnMemory()
    {
    	return m_Info.lowMemory;
    }

    public static String getOsVersion()
    {
    	return System.getProperty("os.version");
    }

    public static String getBuildVersionInc()
    {
    	return android.os.Build.VERSION.INCREMENTAL;
    }

    public static int getBuildVersionSDK()
    {
    	return android.os.Build.VERSION.SDK_INT;
    }

    public static String getBuildDevice()
    {
    	return android.os.Build.DEVICE;
    }

    public static String getBuildModel()
    {
    	return android.os.Build.MODEL;
    }

    public static String getBuildProduct()
    {
    	return android.os.Build.PRODUCT;
    }

    public static String getFilesDir()
    {
        return Main.mainActivity.get().getBaseContext().getFilesDir().getAbsolutePath();
    }

    public static String getDataDir()
    {
        return Main.mainActivity.get().getBaseContext().getDataDir().getAbsolutePath();
    }

    public static String getCacheDir()
    {
        return Main.mainActivity.get().getBaseContext().getCacheDir().getAbsolutePath();
    }

    public static String getObbDir()
    {
        return Main.mainActivity.get().getBaseContext().getObbDir().getAbsolutePath();
    }

    public static String getExternalFilesDir()
    {
        return Main.mainActivity.get().getBaseContext().getExternalFilesDir(null).getAbsolutePath();
    }

    public static String getExternalCacheDir()
    {
        return Main.mainActivity.get().getBaseContext().getExternalCacheDir().getAbsolutePath();
    }

    public static String getPackageName()
    {
    	return Main.mainActivity.get().getApplication().getPackageName();
    }

    public static String getCurrentLanguage()
    {
        String localLanguage = new String(Locale.getDefault().toString());

        if(localLanguage.contains("es_") && !localLanguage.equals("es_ES"))
        {
            return "es-MX";
        }
        else
        {
            return localLanguage;
        }
    }

    public static String getStoreID()
    {
        return "StoreyBorey";
    }

	public static String getCurrentVersionCode()
	{
		int versionCode = 1;
		try
		{
			PackageInfo pInfo = Main.mainActivity.get().getPackageManager().getPackageInfo(getPackageName(), 0);
			versionCode = pInfo.versionCode;
		}
		catch (PackageManager.NameNotFoundException e)
		{
			Log.d("NameNotFoundException", "versionCode");
		}

		return "gp_" + Integer.toString(versionCode);
	}


    public static boolean isConnected()
    {
//        Main context = Main.mainActivity.get();
//        if (context != null)
//        {
//            ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
//
//            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP)
//            {
//                Network[] activeNetworks = cm.getAllNetworks();
//                for (Network n: activeNetworks)
//                {
//                    NetworkInfo nInfo = cm.getNetworkInfo(n);
//                    if(nInfo.isConnected())
//                    {
//                        return true;
//                    }
//                }
//            }
//            else
//            {
//                NetworkInfo[] info = cm.getAllNetworkInfo();
//                if (info != null)
//                {
//                    for (NetworkInfo anInfo : info)
//                    {
//                        if (anInfo.getState() == NetworkInfo.State.CONNECTED)
//                        {
//                            return true;
//                        }
//                    }
//                }
//            }
//
//            return false;
//        }
        return false;
    }

    public static boolean isHttpProxyEnabled()
    {
        String proxyAddress = new String();
        try {
            proxyAddress = System.getProperty("http.proxyHost");
            proxyAddress += ":" + System.getProperty("http.proxyPort");

        } catch (Exception ex) {
            //ignore
        }
        return proxyAddress != null && !proxyAddress.equals("null:null");
    }

    public static String getProxyHost()
    {
        String proxyAddress = new String();
        try {
            proxyAddress = System.getProperty("http.proxyHost");
            proxyAddress += ":" + System.getProperty("http.proxyPort");

        } catch (Exception ex) {
            //ignore
        }

        if (!proxyAddress.equals("null:null"))
        {
            return proxyAddress;
        }
        else
        {
            return "";
        }
    }

	public static boolean isOnWifiConnection()
	{
		Main ctx = Main.mainActivity.get();
    	if (ctx != null)
    	{
			ConnectivityManager connManager = (ConnectivityManager)ctx.getSystemService(CONNECTIVITY_SERVICE);
			NetworkInfo activeNetwork = connManager.getActiveNetworkInfo();
			if (activeNetwork != null && activeNetwork.getType() == ConnectivityManager.TYPE_WIFI) {
				// connected to wifi
				return true;
			}
    	}
		return false;
	}

	public static int getAndroidVersion()
	{
	    return android.os.Build.VERSION.SDK_INT;
	}

	public static boolean isExternalStorageWritable()
    {
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED))
        {
            return true;
        }

        return false;
    }

    public static boolean isPermissionGranted(String permission) {
        // For Android < Android M, self permissions are always granted.
        boolean result = true;

        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M)
        {
            int targetSdkVersion = 0;
            try {
                final PackageInfo info = Main.mainActivity.get().getPackageManager().getPackageInfo(
                        getPackageName(), 0);
                targetSdkVersion = info.applicationInfo.targetSdkVersion;
            } catch (PackageManager.NameNotFoundException e) {
                e.printStackTrace();
            }

            if (targetSdkVersion >= android.os.Build.VERSION_CODES.M) {
                // targetSdkVersion >= Android M, we can
                // use Context#checkSelfPermission
                result = Main.mainActivity.get().checkSelfPermission(permission) == PackageManager.PERMISSION_GRANTED;
            } else {
                // targetSdkVersion < Android M, we have to use PermissionChecker
                result = PermissionChecker.checkSelfPermission(Main.mainActivity.get(), permission) == PermissionChecker.PERMISSION_GRANTED;
            }
        }

        return result;
    }

    public static String getUserAgentString()
    {
        String userAgent = System.getProperty("http.agent");
        return userAgent;
    }

    public static String getApkDir()
    {
        ApplicationInfo appInfo = null;
        PackageManager packMgmr = Main.mainActivity.get().getApplication().getPackageManager();
        try {
            appInfo = packMgmr.getApplicationInfo(getPackageName(), 0);
        } catch (PackageManager.NameNotFoundException e) {
            e.printStackTrace();
            throw new RuntimeException("Unable to locate assets, aborting...");
        }
        return appInfo.sourceDir;
    }

    public static String getUniqueDeviceId()
    {
        return Settings.Secure.getString(Main.mainActivity.get().getContentResolver(), Settings.Secure.ANDROID_ID);
    }

    //Callbacks
    public void onTrimMemory(int level)
    {
        new SystemInfo();
    }

    public void onLowMemory()
    {
        new SystemInfo();
    }

    public void onConfigurationChanged(Configuration newConfig)
    {
        //Do nothing
    }

    public static boolean directoryExist(String path) {
        String [] list;

        //Check path in assets
        try {
            list = Main.mainActivity.get().getBaseContext().getAssets().list(path);
            return (list.length > 0);
        } catch (Exception ex) {
            //ignore
        }

        //Check path in files
        try {
            File folder = new File(path);
            if (folder.exists()) {
                String[] fileList = folder.list();
                return (fileList != null && fileList.length > 0);
            }
        } catch (Exception ex) {
            //ignore
        }

        return false;
    }

    public static boolean fileExist(String path) {

        //Check path in assets
        try {
            InputStream file = Main.mainActivity.get().getBaseContext().getAssets().open(path);
            file.close();
            return true;
        } catch (Exception ex) {
            //ignore
        }

        //Check path in files
        try {
            File file = new File(path);
            if (file.exists()) {
                //Detect that is not a folder
                String[] fileList = file.list();
                if (fileList == null) {
                    return true;
                }

                if (fileList.length == 0) {
                    return true;
                }
            }
        } catch (Exception ex) {
            //ignore
            return false;
        }

        return false;
    }

    public static ArrayList<String> getFilenamesInDirectory(String path, boolean recursive, boolean getFiles, boolean getDirectories) {
        /*utf8string fullPathDirectory;
        if(relativePathDirectory.starts_with(m_RootPath))
        {
            fullPathDirectory = relativePathDirectory;
        }
        else
        {
            fullPathDirectory = m_RootPath + relativePathDirectory;
        }

        if(fullPathDirectory.empty())
        {
            return false;
        }*/

        ArrayList<String> items = new ArrayList<String>();
        String [] list;

        try {
            list = Main.mainActivity.get().getBaseContext().getAssets().list(path);
            if (list.length > 0) {
                for (String file : list) {
                    String fullPath;
                    if(path.isEmpty() || path.endsWith("/")) {
                        fullPath = path + file;
                    }
                    else {
                        fullPath = path + "/" + file;
                    }
                    boolean isDirectory = directoryExist(fullPath);
                    boolean isFile = fileExist(fullPath);

                    if (getFiles && isFile) {
                        items.add(fullPath);
                    }
                    else if (getDirectories && isDirectory) {
                        items.add(fullPath);
                    }

                    if(recursive && isDirectory) {
                        items.addAll(getFilenamesInDirectory(fullPath, recursive, getFiles, getDirectories));
                    }
                }
            }
        } catch (Exception ex) {
            //ignore
        }
        return items;
    }
}
