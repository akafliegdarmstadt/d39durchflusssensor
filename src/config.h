#pragma once

#define SENS_PIN 5      // which pin the sensor is connected to
#define TIKSCONF 8012.0 // ticks per liter

// Bluetooth config
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)
#define ADV_LEN 12