####MiniRacer on Mario Kart

Some pictures of my bot - *MiniRacer*:

![mycam](https://github.com/tanay-bits/tanayChoudhary_ME433_2016/blob/master/HW16-TechCup/mariokart.jpeg)

![mycam2](https://github.com/tanay-bits/tanayChoudhary_ME433_2016/blob/master/HW16-TechCup/miniracer.jpeg)

The Android app thresholds across RGB over 3 bands of 30 rows each from the top, middle, and bottom of the camera's view. The selected pixels have Red content more than the top slider's value, and Green and Blue content less than the middle and lower sliders' values respectively.

The column Center of Mass (CoM) of all selected pixels is calculated, and its offset from the middle (320, since resolution is 640x480) is sent to the PIC. If no pixel is thresholded, the value sent is 0.

The PIC receives the offset (~ error from desired heading) via USB CDC and
+   if it's > ERRORBAND or < -ERRORBAND, makes the bot turn right or left using PD control on each wheel
+   if it's exactly 0 (no CoM), reverses both motors' directions and moves back slowly so that it can attempt to locate the line again.
+   if it's between -ERRORBAND and +ERRORBAND, it makes the bot move straight by turning both motors at high speed.

Kp = 1.0 and Kd = 0.5 seem to work well for me. The bot was successfully able to complete the circuit.