# Build FP/M from sources

### Install cross C/C++ compiler for ARM-32 target

On Ubuntu:

    sudo apt install gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential cmake

On MacOS:

  * Download and install package [arm-gnu-toolchain-14.2.rel1-darwin-x86_64-arm-none-eabi.pkg](https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-darwin-x86_64-arm-none-eabi.pkg).
  * Add directory `/Applications/ArmGNUToolchain/14.2.rel1/arm-none-eabi/bin` to your $PATH.

### Install Pico SDK

    git clone https://github.com/raspberrypi/pico-sdk
    git clone https://github.com/raspberrypi/pico-examples
    cd pico-sdk
    git submodule update --init

Add two lines to your ~/.bashrc script:

    export PICO_SDK_PATH=$HOME/pico-sdk
    export PICO_EXAMPLES_PATH=$HOME/pico-examples

### Checkout and build FP/M from sources

    git clone https://github.com/fp-m/fpm-embedded.git
    cd fpm-embedded
    make

Resulting firmware images for RP2040 are located in pico/build directory:

  * fpm-rp2040-2mb.uf2 - for devices with 2-Mbyte Flash chip
  * fpm-rp2040-8mb.uf2 - for devices with 8-Mbyte Flash chip
  * fpm-rp2040-16mb.uf2 - for devices with 16-Mbyte Flash chip

### Unix demo

To explore the FP/M command-line interface without the need for actual hardware,
you can find the fpm-demo binary within the unix/build directory.
This provides a simulated environment for interacting with FP/M commands.

```
$ cd unix/build
$ ./fpm-demo
Start FP/M on Unix
Use '?' for help or 'exit' to quit.

flash:/ > ver

FP/M version 0.1.213
Git commit 5a64844, built on Jan 20 2025 at 01:44:42
Unix Darwin x86_64 version 24.2.0
Free memory 1024 kbytes

flash:/ > exit
```
