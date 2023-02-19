/*
  defs.h - General parameter definitions
 */
#pragma once

// Pins
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1

#define SERVO_PIN 10

// Stepper pins
#define STEP1 A3
#define STEP2 A1
#define STEP3 A2
#define STEP4 A0

// TLE Server & Query Info
#define HEADER_STR  "ISS (ZARYA)"
#define TLE_LEN     69
#define MAX_BUFFER  1024
#define SERVER      "celestrak.org"
#define QUERY       "/NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE"

// Misc. Flags
#define CHECK_DISPLAY_CONNECTION    false
#define CHECK_COMPASS_CONNECTION    false
#define WAIT_FOR_SERIAL             false
#define DO_PRINT_DEBUG              false

// If set true, will not attempt to automatically point north at startup
// Assumes that pedestal is manually pointed north before startup
#define DO_BYPASS_COMPASS           false

#define MAG_NORTH_LAT           86.494
#define MAG_NORTH_LON           162.867
#define TRUE_NORTH_OFFSET_DEG   -3.73

// Refresh durations
#define TIME_REFRESH_DELAY_MIN 10
#define TLE_REFRESH_DELAY_MIN  60
#define ORBIT_REFRESH_DELAY_MS 500

// Stepper Motor Specs
#define STEPS_PER_REV (2038*4)
#define STEPPER_SPEED 500
#define STEPPER_ACCEL 300

// Servo PWM Range
#define SERVO_MIN_PWM 500
#define SERVO_MAX_PWM 2500

// Timezone used for time display
// const int timeZone = 1;   // Central European Time
const int timeZone = -5;  // Eastern Standard Time (USA)
// const int timeZone = -4;  // Eastern Daylight Time (USA)
// const int timeZone = -8;  // Pacific Standard Time (USA)
// const int timeZone = -7;  // Pacific Daylight Time (USA)
