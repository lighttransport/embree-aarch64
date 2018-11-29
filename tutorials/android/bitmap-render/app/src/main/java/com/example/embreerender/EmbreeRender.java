/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.embreerender;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.graphics.Point;
import android.os.Bundle;
import android.content.Context;
import android.view.View;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.view.Display;
import android.view.WindowManager;

public class EmbreeRender extends Activity
{
    private static final int MY_PERMISSIONS_REQUEST_READ_CONTACTS = 7;
    private boolean okToGo = false;

    @Override
    public void onRequestPermissionsResult(int requestCode,
    	String permissions[], int[] grantResults) {
        switch (requestCode) {
    	case MY_PERMISSIONS_REQUEST_READ_CONTACTS: {
    	    // If request is cancelled, the result arrays are empty.
    	    if (grantResults.length > 0
    		&& grantResults[0] == PackageManager.PERMISSION_GRANTED) {
    		// permission was granted, yay! Do the
    		// contacts-related task you need to do.
                okToGo = true;
    	    } else {
    		// permission denied, boo! Disable the
    		// functionality that depends on this permission.
    	    }
    	    return;
    	}
    
    	// other 'case' lines to check for other
    	// permissions this app might request.
        }
    }

    // Called when the activity is first created.
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Display display = getWindowManager().getDefaultDisplay();
        Point displaySize = new Point();
        display.getSize(displaySize);
        setContentView(new EmbreeRenderView(this, displaySize.x, displaySize.y));
    }

    // load our native library
    static {
        System.loadLibrary("embreerender");
    }
}

// Custom view for rendering scene.
//
// Note: suppressing lint wrarning for ViewConstructor since it is
//       manually set from the activity and not used in any layout.
@SuppressLint("ViewConstructor")
class EmbreeRenderView extends View {
    private Bitmap mBitmap;
    private long mStartTime;

    // implementend by libembreerender.so
    private static native void renderScene(Bitmap  bitmap, long time_ms);

    public EmbreeRenderView(Context context, int width, int height) {
        super(context);
        mBitmap = Bitmap.createBitmap(width, height, Bitmap.Config.RGBA_8888);
        mStartTime = System.currentTimeMillis();
    }

    @Override protected void onDraw(Canvas canvas) {
        renderScene(mBitmap, System.currentTimeMillis() - mStartTime);
        canvas.drawBitmap(mBitmap, 0, 0, null);
        // force a redraw, with a different time-based pattern.
        invalidate();
    }
}
