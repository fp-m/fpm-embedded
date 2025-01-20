# Getting Started

## Prerequisites

To play with FP/M you will need the following:

  * A development board with RP2040 chip. [Raspberry Pi Pico suits](https://www.raspberrypi.com/products/raspberry-pi-pico/) just fine.
    Full list of supported boards is available on [Supported Platforms](docs/Supported-Platforms.md) page.
    Boards with SD card are more fun.
  * A computer with USB port and appropriate cable to connect to USB port of the development board.
  * [Minicom](https://en.wikipedia.org/wiki/Minicom) or any other terminal emulator installed in your computer.
  * Many RP2040 development boards have a slot for micro-SD card.
    You can use any SD card, preformatted to FAT-32 or exFAT filesystem.
    An SD card is not required for the FP/M, but it allows you to easily
    transfer files between the internal Flash file system and an external computer.

## Install FP/M

  * Download file [fpm-rp2040-2mb.uf2](https://github.com/fp-m/fpm-embedded/releases/download/v0.1.0/fpm-rp2040-2mb.uf2).
    This image is suitable for all development boards.
    It provides about 2 Mbytes of space on Flash filesystem.
    In case you have a Flash chip of larger size, images `fpm-rp2040-8mb.uf2`
    and ``fpm-rp2040-16mb.uf2` are also available.
  * Upload the firmware image onto your board. There are two ways:
      * Press and hold BOOTSEL button, and power on or reset the board.
        The RP2040 now runs the RP2040 factory bootloader.
        An USB drive named RPI-RP2 should appear on your computer.
        Copy the .uf2 firmware file onto the USB drive.
        Eject the USB drive. At this step, the firmware should be uploaded and running.
      * In case you have a `picotool` utility installed on your computer,
        use: "picotool load -f -x fpm-rp2040-2mb.uf2".
  * Connect via USB to the console port at speed 115200 bit/sec.
    Use minicom or any other terminal emulator.
    I recommend a color mode.
    Name of virtual serial port varies on different platforms.
    On my machine the required command is "minicom -con -D /dev/tty.usbmodem142101".
    Press Enter. You should see prompt "flash:/ >".

## Internal commands

Type `help` and press Enter. You will see:
```
flash:/ > help
FP/M built-in commands are:
cat or type     Display the contents of a text file
cd              Show or change current directory
clear or cls    Clear the console screen
cp or copy      Copy files or directories
date            Show or change the system date
echo            Copy text directly to the console output
eject           Release removable disk device
format          Create filesystem on a disk device
help or ?       Show all built-in commands
ls or dir       List the contents of a directory
mkdir           Create a directory
mount           Engage removable disk device
mv or rename    Rename or move files and directories
reboot          Restart the FP/M kernel
rm or erase     Delete a file or set of files
rmdir           Remove a directory
time            Set or show the current system time
ver             Show the version of FP/M software
vol             Show the volume label of a disk device
exit            Close down the command interpreter

Enter 'command -h' for more information on any of the above commands.
```
These commands are built into the command interpreter and always available.

## External commands

Type `ls /bin`. You get a list of executables which reside in Flash memory.
```
flash:/ > ls bin
cmd.exe      free.exe     hello.exe    printf.exe
```

Try hello.exe:
```
flash:/ > hello
Hello, World!
```

External commands can be executed only from `flash:/bin` directory.
You can put more *.exe binaries there by copying from SD card.

## History

Previous command can be recalled from history by Up Arroy key.
The command line can be edited in place. Use Left/Right arrows
and Home/End keys to navigate. ALternatively, use ^B/^F/^H/^E keys.
