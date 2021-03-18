<link rel="stylesheet" href="styles.css">
[Home](https://ramidabit.github.io/ultrabucket/){: .btn}
[Parts](https://ramidabit.github.io/ultrabucket/parts){: .btn}


## Weekly Updates

### February 26, 2021
- Developed idea with potential addition of two to three ultrasonic sensors and use of stereo sound for directional distance sensing.
  - Ordered addional ultrasonic sensors:
    ![Image](https://i.imgur.com/iWE1APt.png)
- Refined sound delivery method to use DAC, I2C, and I2S with onboard stereo jack rather than bluetooth.
- Updated parts list and block diagram to reflect above changes.
- Imported initial work with ultrasonic sensor and onboard gyroscope.

### March 5, 2021
- Got toggling functionality working using gyroscope and one ultrasonic sensor.
  - In the process of reading distance on multiple channels
- Some work done regarding initialization of audio jack.
- Made plans to acquire hat which will house components.

### March 12, 2021
- Achieved dual channel distance reading using two ultrasonic sensors.
   - Added a low-power state by disabling clocks when no reading is desired.
     ![Image](https://i.imgur.com/KZES5GR.png)
- Output some static noise by interfacing with audio jack.
- Acquired hat and LED strip for plans to add an active/inactive indicator.

### March 17, 2021 (Final Submission):
- Added third ultrasonic sensor to measure distance directly in front of the user.
   - The other two sensors are placed at 45 degree angles to the left and right.
- Changed trigger to be the same for all ultrasonic sensors.
- Moved ultrasonic sensors off of PB6 and PB7 because they are used for I2C.
   - Echo pins now connected to PA0, PA1, and PA2.
- Got quick beeps to play through the audio jack using I2C and I2S.
- Added LED strip to indicate when the device is in "active" mode, connecting the data line via PWM.
