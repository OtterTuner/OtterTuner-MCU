# OtterTuner-MCU

## Hardware Used:

- ESP32 Tiny S3

This project has several modules composing different features of the final prototype (`mcu` folder):

- Autocorrelation.ino: Frequency detection algorithm that takes in samples of ADC and uses autocorrelation to measure a frequency
- BLE.ino: Bluetooth Low Energy module that allows for communication from the mobile app to the ESP32
- Button.ino: Code for buttons
- LED.ino: Module for LED animations
- Motor.ino: Motor control software
- PID.ino: Proportion Integration Derivative module that adjusts how much power to give the motor given frequency.
- mcu.ino: Main file that includes multithreaded

The `test` folder was created to test individual modules including autocorrelation, motor control, and PID control.
The `initial_prototype_mcu` folder was the first iteration of the OtterTuner used for the first term of capstone.
