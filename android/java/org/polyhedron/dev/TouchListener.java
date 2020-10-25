package org.polyhedron.dev;

import android.view.MotionEvent;
import android.view.View;

public class TouchListener implements View.OnTouchListener
{
    private static native void nativeTouchesBegin(int id, float x, float y);
    private static native void nativeTouchesEnd(int id, float x, float y);
    private static native void nativeTouchesMove(int[] id, float[] x, float[] y);
    private static native void nativeTouchesCancel(int[] id, float[] x, float[] y);
    private static native void nativeToggleDebugMenu();

    @Override
    public boolean onTouch(View view, MotionEvent motionEvent) {
        final int pointerNumber = motionEvent.getPointerCount();
        final int[] ids = new int[pointerNumber];
        final float[] xs = new float[pointerNumber];
        final float[] ys = new float[pointerNumber];

        for (int i = 0; i < pointerNumber; i++) {
            ids[i] = motionEvent.getPointerId(i);
            xs[i] = motionEvent.getX(i);
            ys[i] = motionEvent.getY(i);
        }

        switch(motionEvent.getAction() & MotionEvent.ACTION_MASK)
        {
            case MotionEvent.ACTION_POINTER_DOWN:
//                if (BuildConfig.IS_DEBUG)
//                {
//                    nativeToggleDebugMenu();
//                }
                break;

            case MotionEvent.ACTION_DOWN:
                final int idDown = ids[0];
                final float xDown = xs[0];
                final float yDown = ys[0];

                nativeTouchesBegin(idDown, xDown, yDown);
                break;

            case MotionEvent.ACTION_MOVE:
                nativeTouchesMove(ids, xs, ys);
                break;

            case MotionEvent.ACTION_POINTER_UP:
                final int idPointerUp = ids[0];
                final float xPointerUp = xs[0];
                final float yPointerUp = ys[0];
                //nativeTouchesEnd(idPointerUp, xPointerUp, yPointerUp);
                //nativeTouchesCancel(idPointerUp, xPointerUp, yPointerUp);

                break;

            case MotionEvent.ACTION_UP:
                final int idUp = ids[0];
                final float xUp = xs[0];
                final float yUp = ys[0];

                nativeTouchesEnd(idUp, xUp, yUp);
                break;

            case MotionEvent.ACTION_CANCEL:
                nativeTouchesCancel(ids, xs, ys);
                break;
        }

        return true;
    }
}
