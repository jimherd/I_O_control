## Update Pi the Robot animatronic head with new technology
----

Add RP2040 Pico board to manage head hardware

* Head servo motors
* Neck stepper motor
* Neopixel RGB LEDs

### New hardware

* Servo motor control
    - 16 channel Adafruit PCA9685 board
* Stepper motor control
    - TMC2208 stepper motor board
* Neopixel channel

### Tools

* Windows 10 laptop
    * Visual Studio Code
        * Raspberry Pi Pico extension (Oct 24)
        * SDK 2.0.0 (Oct 24)
        * Arm GNU toolchain: 13_2_Rel1 (Oct 24)
        * FreeRTOS V11.1.0 (Oct 24)
    * Ninja V1.12.1
    * Cmake V3.28.6
    * OpenOCD
    * Git for windows
    * Python 3.11
    * Picotools
* Hardware PicoProbe debug probe : CMSIS version


Jim Herd November 2024
