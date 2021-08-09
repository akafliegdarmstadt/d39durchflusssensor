# NMEA Fuel-flow Sensor

This repository holds the software and documentation for the onboard fuel-flow instrumentation of our
Touring Motor Glider D-39.

## Concept

The actual sensor outputs a square wave whose frequency corresponds to fuel-flow.

We use a ESP32 to read the sensor and present the data as an NMEA-Sentence over Bluetooth LE.

The sentence is of the form `$PFLO,xxx.x*??`, where xxx.x is the number of ticks per second and ?? the checksum.

Our calibration factor is 8600 ticks per liter.

## Setup Build Environment

* zephyr development environment setup as described in https://docs.zephyrproject.org/1.14.0/getting_started/installation_linux.html
* install esp32 specific toolchan as described in https://docs.zephyrproject.org/latest/boards/xtensa/esp32/doc/index.html

## Build Application

Create a build folder and run cmake before building the application using ninja.

```bash
mkdir build && cd build
cmake -GNinja ..
ninja
```

## Flash Application

Assuming esp32 is connected via /dev/ttyUSB0 use

```bash
ninja flash
```

Otherwise the esp device path has to be set during build.
