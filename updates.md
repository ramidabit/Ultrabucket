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
- Output some static noise by interfacing with audio jack.
- Acquired hat and LED strip for plans to add an active/inactive indicator.
