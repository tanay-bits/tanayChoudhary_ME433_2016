package me433.tanay.mycam;

import android.app.Activity;
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

public class MainActivity extends Activity implements TextureView.SurfaceTextureListener {
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

    static long prevtime = 0; // for FPS calculation
    static int thresh1 = 53;  // for getting threshold from slider
    static int thresh2 = 188;  // for getting threshold from slider
    static int thresh3 = 68;  // for getting threshold from slider
    static int dist = 0;    // px dist from center (320)

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
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

        setMyControlListener();
    }

    public void setMyControlListener() {
        myControl1.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                thresh1 = progress;
                sliderTextView.setText("RGB thresholds are: "+thresh1+" "+thresh2+" "+thresh3);
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

            for (int n = 0; n < 440; n = n+150) {
                for (int j = n; j < n + 50; j++) {
                    int[] thresholdedColors = new int[bmp.getWidth()];
                    int[] pixels_mid = new int[bmp.getWidth()];
                    int startYmid = 30 + j; // which row in the bitmap to analyse to read
                    bmp.getPixels(pixels_mid, 0, bmp.getWidth(), 0, startYmid, bmp.getWidth(), 1);

                    int[] thresholdedPixels_mid = new int[bmp.getWidth()];

                    for (int i = 0; i < bmp.getWidth(); i++) {
                        if ((red(pixels_mid[i]) > thresh1) && (green(pixels_mid[i]) < thresh2) && (blue(pixels_mid[i]) < thresh3)) {
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
            }

            int COM_mid;
            boolean midFlag;
            //watch out for divide by 0
            if (wbTotal_mid<=0) {
                COM_mid = bmp.getWidth()/2;
                midFlag = false;
                dist = 0;
            }
            else {
                COM_mid = wbCOM_mid/wbTotal_mid;
                midFlag = true;
                dist = COM_mid - 320;

//                String sendString = String.valueOf(dist + "\n");
//                try {
//                    sPort.write(sendString.getBytes(),10); // 10 is the timeout (error)
//                }
//                catch (IOException e) {}
            }

            // draw a circle where you think the COM is
            // also write the value as text
            if (midFlag) {
                canvas.drawCircle(COM_mid, 185, 5, paint1);
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