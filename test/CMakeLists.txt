# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.13.1)

set(BOARD qemu_cortex_m3)

find_package(Zephyr)
project(d39_fuel_flow)

# Add test code
target_sources(app PRIVATE src/main.c ../src/nmea.c)

target_include_directories(app PRIVATE ../src)
