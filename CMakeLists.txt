# Find Zephyr. This also loads Zephyr's build system.
cmake_minimum_required(VERSION 3.13.1)

option(TESTS "Run tests in test/" ON)

set(BOARD esp32)

find_package(Zephyr)
project(d39_fuel_flow)

if(TESTS)
    # Add test code
    target_sources(app PRIVATE test/main.c src/nmea.c)

    target_include_directories(app PRIVATE src)
else()
    # Add application code
    target_sources(app PRIVATE src/main.c src/nmea.c)
endif()