# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.13.1)

set(BOARD esp32)

find_package(Zephyr)
project(d39_fuel_flow)

# Add application code
target_sources(app PRIVATE 
                src/main.c
                src/nmea.c
                src/d39_gpio/d39_gpio.c
                src/d39_bt/d39_bt.c)
target_include_directories(app PRIVATE src)
