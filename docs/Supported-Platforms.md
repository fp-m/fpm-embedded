# Supported Platforms

The following is a list of platforms and boards supported in FP/M.

  * Raspberry Pi Pico with RP2040
  * Raspberry Pi Pico 2 with RP2350
  * Unix demo

## Raspberry Pi Pico with RP2040

Boards with SD card:

  * [SparkFun Thing Plus - RP2040](https://learn.sparkfun.com/tutorials/rp2040-thing-plus-hookup-guide/hardware-overview)
  * [Challenger RP2040 SD/RTC](https://ilabs.se/challenger-rp2040-sd-rtc-datasheet)
  * [Waveshare RP2040-PiZero](https://www.waveshare.com/wiki/RP2040-PiZero)
  * [HackyPi](https://shop.sb-components.co.uk/products/hackypi-compact-diy-usb-hacking-tool)
  * [MusicPi](https://shop.sb-components.co.uk/products/musicpi-high-quality-stereo-audio)
  * [ArdiPi](https://shop.sb-components.co.uk/products/ardipi-uno-r3-alternative-board-based-on-pico-w)
  * [Olimex RP2040-PICO-PC](https://www.olimex.com/Products/RaspberryPi/PICO/RP2040-PICO-PC/open-source-hardware)

Boards without SD card:

  * [Raspberry Pi Pico](https://www.raspberrypi.com/products/raspberry-pi-pico/)
  * [Raspberry Pi Pico W](https://www.raspberrypi.com/products/raspberry-pi-pico/)
  * [Arduino Nano RP2040 Connect](https://store-usa.arduino.cc/products/arduino-nano-rp2040-connect)
  * [Waveshare RP2040-Plus](https://www.waveshare.com/wiki/RP2040-Plus)
  * [Waveshare RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero)
  * [Waveshare RP2040-Matrix](https://www.waveshare.com/wiki/RP2040-Matrix)
  * [Waveshare Pico Eval Board](https://www.waveshare.com/wiki/Pico-Eval-Board)
  * [YD-RP2040](https://circuitpython.org/board/vcc_gnd_yd_rp2040/) by VCC-GND Studios

Flash memory size:

RP2040 Board                 | Flash size | SD card | Display | WiFi
-----------------------------|------------|---------|---------|-----
SparkFun Thing Plus - RP2040 | 16 Mbytes  | yes     | ---     | ---
Challenger RP2040 SD/RTC     | 8 Mbytes   | yes     | ---     | ---
Waveshare RP2040-PiZero      | 16 Mbytes  | yes     | ---     | ---
HackyPi                      | 2 Mbytes   | yes     | 240x135 | ---
MusicPi                      | ---        | yes     | 240x135 | ---
ArdiPi                       | 2 Mbytes   | yes     | ---     | ---
Olimex RP2040-PICO-PC        | ---        | yes     | ---     | ---
Raspberry Pi Pico            | 2 Mbytes   | no      | ---     | ---
Raspberry Pi Pico W          | 2 Mbytes   | no      | ---     | CYW43439
Arduino Nano RP2040 Connect  | 16 Mbytes  | no      | ---     | NINA-W102
Waveshare RP2040-Plus        | 16 Mbytes  | no      | ---     | ---
Waveshare RP2040-Zero        | 2 Mbytes   | no      | ---     | ---
Waveshare RP2040-Matrix      | 2 Mbytes   | no      | ---     | ---
Waveshare Pico Eval Board    | ---        | unusable| 480×320 | ---
YD-RP2040                    | 2 Mbytes   | no      | ---     | ---

## Raspberry Pi Pico 2 with RP2350

Boards with SD card:

  * [Waveshare RP2350-GEEK](https://www.waveshare.com/wiki/RP2350-GEEK)

Boards without SD card:

  * [Raspberry Pi Pico 2](https://www.raspberrypi.com/products/raspberry-pi-pico-2/)
  * [Raspberry Pi Pico 2 W](https://www.raspberrypi.com/products/raspberry-pi-pico-2/)
  * [Waveshare RP2350-Zero](https://www.waveshare.com/wiki/RP2350-Zero)

Flash memory size:

RP2350 Board                 | Flash size | SD card | Display | WiFi
-----------------------------|------------|---------|---------|-----
Waveshare RP2040-GEEK        | 16 Mbytes  | yes     | 240x135 | ---
Raspberry Pi Pico            | 4 Mbytes   | no      | ---     | ---
Raspberry Pi Pico W          | 4 Mbytes   | no      | ---     | CYW43439
Waveshare RP2040-Zero        | 2 Mbytes   | no      | ---     | ---

## Unix

If you don't have a board available, FP/M has a demo that you can run on Linux or MacOS.
