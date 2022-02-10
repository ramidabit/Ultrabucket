# Sensor and Peripheral Interface Design Project
## The Ultrabucket Pro  |  [Link to project page](https://ramidabit.github.io/ultrabucket/)

Authors:

- Rami Dabit

- Kyle Kam


--------------------------------------------------

# Abstract:
For people with visual impairments, a white cane or other ground-level device is often their only means of measuring depth. The Ultrabucket Pro offers an alternate avenue for depth perception, and is designed to prevent the wearer from walking into objects closer to the head. Using an ultrasonic distance sensor to monitor the distance in front of the user, the hat will output a series of beeps with frequency inversely proportional to the distance such that the wearer is informed if they are nearing danger. As the user gets closer to an object, higher pitched beeps will be transmitted to a bluetooth audio device. If there is too much distance remaining, then no noise will be played. The functionality can be turned on and off by a quick spin of the hat, making use of the onboard gyroscope to measure the spin and determine whether it crosses a certain threshold.

# View [project page](https://ramidabit.github.io/ultrabucket/) for more

Many thanks to Dr. Yoga Isukapalli for his help throughout this project


--------------------------------------------------

# Changelog
## February 26, 2021
- Imported prior work with ultrasonic sensor and gyroscope

## March 5, 2021
- Got toggling functionality working using gyroscope reading
- Audio work to be pushed

## March 12, 2021
- Added dual channel ultrasonic sensor reading
- Got static to output through audio jack

## March 17, 2021
- Added third ultrasonic sensor initalization
- Moved ultrasonic sensors from PB6 and PB7 to PA0, PA1, and PA2
- Connected audio files and added LED strip interface (PWM)
