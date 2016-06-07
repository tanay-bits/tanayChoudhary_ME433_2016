/* Copyright 2011-2013 Google Inc.
 * Copyright 2013 mike wakerly <opensource@hoho.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * Project home page: https://github.com/mik3y/usb-serial-for-android
 */

package com.hoho.android.usbserial.examples;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.util.Log;
import android.view.WindowManager;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ScrollView;
import android.widget.TextView;

import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.util.HexDump;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.SeekBar;
import java.io.IOException;
import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;
import static android.graphics.Color.rgb;

/**
 * Monitors a single {@link UsbSerialPort} instance, showing all data
 * received.
 *
 * @author mike wakerly (opensource@hoho.com)
 */
public class SerialConsoleActivity extends Activity implements TextureView.SurfaceTextureListener {

    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(640,480,Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;

    SeekBar myControl1;
    SeekBar myControl2;
    SeekBar myControl3;
    TextView sliderTextView;

//    TextView picTextView;

    static long prevtime = 0; // for FPS calculation
    static int thresh1 = 203;  // for getting threshold from slider
    static int thresh2 = 90;  // for getting threshold from slider
    static int thresh3 = 9;  // for getting threshold from slider
//    static int COM_mid = 0;

    private final String TAG = SerialConsoleActivity.class.getSimpleName();

    /**
     * Driver instance, passed in statically via
     * {@link #show(Context, UsbSerialPort)}.
     *
     * <p/>
     * This is a devious hack; it'd be cleaner to re-create the driver using
     * arguments passed in with the {@link #startActivity(Intent)} intent. We
     * can get away with it because both activities will run in the same
     * process, and this is a simple demo.
     */
    private static UsbSerialPort sPort = null;

//    private TextView mTitleTextView;
//    private TextView mDumpTextView;
//    private ScrollView mScrollView;
//    private CheckBox chkDTR;
//    private CheckBox chkRTS;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    private SerialInputOutputManager mSerialIoManager;

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {

        @Override
        public void onRunError(Exception e) {
            Log.d(TAG, "Runner stopped.");
        }

        @Override
        public void onNewData(final byte[] data) {
            SerialConsoleActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    SerialConsoleActivity.this.updateReceivedData(data);
                }
            });
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.serial_console);
//        mTitleTextView = (TextView) findViewById(R.id.demoTitle);
//        mDumpTextView = (TextView) findViewById(R.id.consoleText);
//        mScrollView = (ScrollView) findViewById(R.id.demoScroller);
//        chkDTR = (CheckBox) findViewById(R.id.checkBoxDTR);
//        chkRTS = (CheckBox) findViewById(R.id.checkBoxRTS);

//        chkDTR.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
//            @Override
//            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
//                try {
//                    sPort.setDTR(isChecked);
//                }catch (IOException x){}
//            }
//        });
//
//        chkRTS.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
//            @Override
//            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
//                try {
//                    sPort.setRTS(isChecked);
//                }catch (IOException x){}
//            }
//        });

        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
        mSurfaceHolder = mSurfaceView.getHolder();

        mTextureView = (TextureView) findViewById(R.id.textureview);
        mTextureView.setSurfaceTextureListener(this);

        mTextView = (TextView) findViewById(R.id.cameraStatus);

        paint1.setColor(0xffff0000); // red
        paint1.setTextSize(24);

        myControl1 = (SeekBar) findViewById(R.id.seek1);
        myControl2 = (SeekBar) findViewById(R.id.seek2);
        myControl3 = (SeekBar) findViewById(R.id.seek3);

        sliderTextView = (TextView) findViewById(R.id.sliderStatus);
        sliderTextView.setText("Enter RGB thresholds");

//        picTextView = (TextView) findViewById(R.id.picReader);
//        picTextView.setText("Incoming value from PIC");

        setMyControlListener();

    }

    //STUFF THAT WAS ALREADY HERE:

    @Override
    protected void onPause() {
        super.onPause();
        stopIoManager();
        if (sPort != null) {
            try {
                sPort.close();
            } catch (IOException e) {
                // Ignore.
            }
            sPort = null;
        }
        finish();
    }

    void showStatus(TextView theTextView, String theLabel, boolean theValue){
        String msg = theLabel + ": " + (theValue ? "enabled" : "disabled") + "\n";
        theTextView.append(msg);
    }

    @Override
    protected void onResume() {
        super.onResume();
        Log.d(TAG, "Resumed, port=" + sPort);
        if (sPort == null) {
//            mTitleTextView.setText("No serial device.");
        } else {
            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);

            UsbDeviceConnection connection = usbManager.openDevice(sPort.getDriver().getDevice());
            if (connection == null) {
//                mTitleTextView.setText("Opening device failed");
                return;
            }

            try {
                sPort.open(connection);
                sPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

//                showStatus(mDumpTextView, "CD  - Carrier Detect", sPort.getCD());
//                showStatus(mDumpTextView, "CTS - Clear To Send", sPort.getCTS());
//                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
//                showStatus(mDumpTextView, "DTR - Data Terminal Ready", sPort.getDTR());
//                showStatus(mDumpTextView, "DSR - Data Set Ready", sPort.getDSR());
//                showStatus(mDumpTextView, "RI  - Ring Indicator", sPort.getRI());
//                showStatus(mDumpTextView, "RTS - Request To Send", sPort.getRTS());

            } catch (IOException e) {
                Log.e(TAG, "Error setting up device: " + e.getMessage(), e);
//                mTitleTextView.setText("Error opening device: " + e.getMessage());
                try {
                    sPort.close();
                } catch (IOException e2) {
                    // Ignore.
                }
                sPort = null;
                return;
            }
//            mTitleTextView.setText("Serial device: " + sPort.getClass().getSimpleName());
        }
        onDeviceStateChange();
    }

    private void stopIoManager() {
        if (mSerialIoManager != null) {
            Log.i(TAG, "Stopping io manager ..");
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if (sPort != null) {
            Log.i(TAG, "Starting io manager ..");
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener);
            mExecutor.submit(mSerialIoManager);
        }
    }

    private void onDeviceStateChange() {
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {
//        final String message = "Incoming value from PIC: "
//                + HexDump.dumpHexString(data) + "\n\n";
//        mDumpTextView.append(message);
//        mScrollView.smoothScrollTo(0, mDumpTextView.getBottom());

//        byte[] sData = {'a',0}; try { sPort.write(sData, 10); } catch (IOException e) { }

//        int i = 1; int j = 2; // the two numbers to send
//        String sendString = String.valueOf(i) + " " + String.valueOf(j);
//        try {
//            sPort.write(sendString.getBytes(),10); // 10 is the timeout (error)
//        }
//        catch (IOException e) {}

//        picTextView.setText(message);
    }

    /**
     * Starts the activity, using the supplied driver instance.
     *
     * @param context
     * @param driver
     */
    static void show(Context context, UsbSerialPort port) {
        sPort = port;
        final Intent intent = new Intent(context, SerialConsoleActivity.class);
        intent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NO_HISTORY);
        context.startActivity(intent);
    }

    // MY SHIT:

    public void setMyControlListener() {
        myControl1.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                thresh1 = progress;
                sliderTextView.setText("RGB thresholds are: "+thresh1+" "+thresh2+" "+thresh3);

//                String sendString = String.valueOf(thresh1 + "\n");
//                try {
//                    sPort.write(sendString.getBytes(),10); // 10 is the timeout (error)
//                }
//                catch (IOException e) {}

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        myControl2.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                thresh2 = progress;
                sliderTextView.setText("RGB thresholds are: "+thresh1+" "+thresh2+" "+thresh3);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        myControl3.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                thresh3 = progress;
                sliderTextView.setText("RGB thresholds are: "+thresh1+" "+thresh2+" "+thresh3);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        parameters.setColorEffect(Camera.Parameters.EFFECT_NONE); // black and white
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
//        parameters.setAutoWhiteBalanceLock(true);   // no auto white-balancing
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    // the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        // Invoked every time there's a new Camera preview frame
        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {

            int wbTotal_mid = 0; // total mass
            int wbCOM_mid = 0; // total (mass time position)

            for (int j = 0; j < 50; j++) {
                int[] thresholdedColors = new int[bmp.getWidth()];
                int[] pixels_mid = new int[bmp.getWidth()];
                int startYmid = 160 + j; // which row in the bitmap to analyse to read
                bmp.getPixels(pixels_mid, 0, bmp.getWidth(), 0, startYmid, bmp.getWidth(), 1);

                int[] thresholdedPixels_mid = new int[bmp.getWidth()];

                for (int i = 0; i < bmp.getWidth(); i++) {
                    if ((red(pixels_mid[i]) > thresh1) && (green(pixels_mid[i]) > thresh2) && (blue(pixels_mid[i]) > thresh3)) {
                        thresholdedPixels_mid[i] = 255;
                        thresholdedColors[i] = rgb(0, 255, 0);
                    } else {
                        thresholdedPixels_mid[i] = 0;
                        thresholdedColors[i] = rgb(0, 0, 0);
                    }
                    wbTotal_mid = wbTotal_mid + thresholdedPixels_mid[i];
                    wbCOM_mid = wbCOM_mid + thresholdedPixels_mid[i] * i;
                }
                bmp.setPixels(thresholdedColors, 0, bmp.getWidth(), 0, startYmid, bmp.getWidth(), 1);
            }
            int COM_mid;
            boolean midFlag;
            //watch out for divide by 0
            if (wbTotal_mid<=0) {
                COM_mid = bmp.getWidth()/2;
                midFlag = false;
            }
            else {
                COM_mid = wbCOM_mid/wbTotal_mid;
                midFlag = true;
            }

            // draw a circle where you think the COM is
            // also write the value as text
            if (midFlag) {
                canvas.drawCircle(COM_mid, 165, 5, paint1);
                canvas.drawText("COM mid = " + COM_mid, 10, 230, paint1);
            }

            c.drawBitmap(bmp, 0, 0, null);
            mSurfaceHolder.unlockCanvasAndPost(c);

            // calculate the FPS to see how fast the code is running
            long nowtime = System.currentTimeMillis();
            long diff = nowtime - prevtime;
            mTextView.setText("FPS " + 1000/diff);
            prevtime = nowtime;
        }
    }

}