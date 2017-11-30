# Quake3

This file contains some general notes for running Q3, Q3Arena and OpenArena on various platforms, including the Raspberry Pi.

On the Raspberry Pi, ioquake3  seems to perform best when run from a virtual terminal without the XServer running in the background.  The easiest way to boot without X starting is to run the command raspi-config and disable X from starting, then reboot the Pi.

On Ubuntu 17.10, the easiest method to stop the (not) XServer is to issue a 'sudo init 3' command, which will restart the system to a virtual terminal.

## Raspberry Pi Install Notes
TODO

## Benchmarking Notes

### How to Benchmark

To benchmark the system, press the ~ key to open the console and type the following:
* timedemo 1
* demo four

After the demo is complete, press the ~ again to see the time and FPS results.  If demo 'four' is not on the system, press the ~ key again and select any available demo listed in "Demos".

### Current Results

Wyatt's Computer - 11.4 secs and !113 FPS
Raspberry Pi 2   - ~16 secs and ~84 FPS (when run from X)
                 - ~14 secs and ~88 FPS (when run from a terminal w/o X)
TV-Desktop 17.10 - 25.4 secs and 49.7 FPS (when run at 640x480 w/ X)
                 - Currently unable to start Q3 from a VT


## Dedicated Server Notes
TODO

## Q3 Console Notes

timedemo 0/1 - Enables/Disables the timedemo feature
demo <demo-name> - runs a demo
cg_drawFPS 0/1 - Enables/Disables the FPS counter in the top right corner of the screen

## Useful Links

[General Linux Setup](http://www.andersaaberg.dk/2012/installing-quake-iii-arena-for-linux-especially-x64-bit/)
[Quake2 Setup](https://github.com/jdolan/quake2)
[Quake1 Setup](https://ubuntuforums.org/showthread.php?t=537039)
[Quake3 Dedicated Server - Docker Image](https://hub.docker.com/r/inanimate/quake3/)
[Debian Quake3 Server Package](https://packages.debian.org/stable/games/quake3-server)

