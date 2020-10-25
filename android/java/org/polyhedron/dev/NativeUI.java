package org.polyhedron.dev;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Message;
import android.util.Log;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import org.polyhedron.dev.R;
import static org.polyhedron.dev.Main.TAG;

public class NativeUI
{
	protected View m_ProgressDialog = null;
	private static NativeUI m_Instance = null;

	public static native void nativeMessageBoxResponsed(String title, String button);
	public static native void nativeFreezeDialogPressed();
	public static native void nativeRunNativeThreadRunners();

	//Household stuff
	NativeUI()
    {
        if (m_Instance == null)
        {
            m_Instance = this;
        }
    }

    private static Main getActivity()
    {
        return Main.mainActivity.get();
    }

    public static NativeUI Instance()
    {
        if (m_Instance == null)
        {
            m_Instance = new NativeUI();
        }
        return m_Instance;
    }

	public void showProcessDialog()
	{
        if (m_ProgressDialog == null)
        {
            Log.d(TAG, "Creating ProgressDialog");
            m_ProgressDialog = (View)getActivity().findViewById(R.layout.activity_indicator);
        }
        else
        {
            Log.d(TAG, "Showing ProgressDialog");
        }
	    View progDialog = getActivity().findViewById(R.layout.activity_indicator);
	    if (progDialog != null)
        {
            progDialog.setVisibility(View.VISIBLE);
        }
	}

	public void dismissCurrentProgressDialog()
	{
	    View progDialog = getActivity().findViewById(R.layout.activity_indicator);
        if (progDialog != null)
        {
            progDialog.setVisibility(View.GONE);
            Log.d(TAG, "Hiding ProgressDialog");
        }
        else
        {
            Log.w(TAG, "Hiding ProgressDialog: missing View!");
        }
	}

	public void createProgressDialog()
    {
        if (m_ProgressDialog == null)
        {
            Log.d(TAG, "Creating ProgressDialog");
            m_ProgressDialog = (View)getActivity().findViewById(R.layout.activity_indicator);
        }
    }

	public void displayMessage(String message)
	{
		if(message != null)
		{
			Toast.makeText(getActivity(), message, Toast.LENGTH_SHORT).show();
		}
	}

    public void showDialog(String title, String message)
    {
    	Dialog dialog = new AlertDialog.Builder(getActivity(), android.R.style.Theme_Material_Light_Dialog)
		    .setTitle(title)
		    .setMessage(message)
		    .setPositiveButton("Ok",
			    new DialogInterface.OnClickListener()
			    {
			    	public void onClick(DialogInterface dialog, int whichButton) {}
			    })
			.create();

	    dialog.show();
    }

    //Api
    public static void triggerMainThreadCallbacks()
    {
        if (getActivity() != null)
        {
            getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    nativeRunNativeThreadRunners();
                }
            });
        }
    }
	public static void displayMessageStatic(String _message)
    {
		if (getActivity() != null)
		{
		    final String message = _message;
		    getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
        		    Instance().displayMessage(message);
                }
            });
		}
	}

	public static void showActivityIndicatorStatic()
	{
		if (getActivity() != null)
		{
		    getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
		            Instance().showProcessDialog();
                }
            });
		}
	}

	public static void hideActivityIndicatorStatic()
	{
		if (getActivity() != null)
		{
		    getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
    		        Instance().dismissCurrentProgressDialog();
                }
            });
		}
	}

    public static void showMessageBox(String title, String message, String okButton, String cancelButton, String thirdButton)
    {
        final String _title = title;
        final String _message = message;
        final String _okButton = okButton;
        final String _cancelButton = cancelButton;
        final String _thirdButton = thirdButton;

		if (getActivity() != null)
		{
		    getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    AlertDialog.Builder dialog = new AlertDialog.Builder(getActivity())
                        .setTitle(_title)
                        .setMessage(_message);

                    if(!_okButton.isEmpty())
                    {
                        dialog.setPositiveButton(_okButton,
                            new DialogInterface.OnClickListener()
                            {
                                public void onClick(DialogInterface dialog, int whichButton)
                                {
                                    nativeMessageBoxResponsed(_title, _okButton);
                                }
                            }
                        );
                    }

                    if (!_cancelButton.isEmpty())
                    {
                        dialog.setNegativeButton(_cancelButton,
                            new DialogInterface.OnClickListener()
                            {
                                public void onClick(DialogInterface dialog, int whichButton)
                                {
                                    nativeMessageBoxResponsed(_title, _cancelButton);
                                }
                            }
                        );
                    }

                    if (!_thirdButton.isEmpty())
                    {
                        dialog.setNeutralButton(_thirdButton,
                            new DialogInterface.OnClickListener()
                            {
                                public void onClick(DialogInterface dialog, int whichButton)
                                {
                                    nativeMessageBoxResponsed(_title, _thirdButton);
                                }
                            }
                        );
                    }

                    dialog.setCancelable(false);
                    dialog.show();
                 }
            });
		}
    }

    public static void showBrowser(String _url)
    {

		if (getActivity() != null)
		{
		    final String url = _url;
		    getActivity().runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    Intent i = new Intent(Intent.ACTION_VIEW);
                    i.setData(Uri.parse(url));
                    Context ctx = getActivity();
                    if( ctx != null ){
                        ctx.startActivity(i);
                    }
                 }
            });
		}
	}

//	public static void showFreezeDialogStatic(String text, String button)
//    {
//		if (getActivity() != null)
//		{
//		    final String _text = text;
//		    final String _button = button;
//		    getActivity().runOnUiThread(new Runnable() {
//                @Override
//                public void run() {
//    		        Instance().showFreezeDialog(_text, _button);
//                }
//            });
//		}
//
//    }
}
