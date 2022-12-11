#pragma once
#define SPIWIFI       SPI  // The SPI port
#define SPIWIFI_SS    13   // Chip select pin
#define ESP32_RESETN  12   // Reset pin
#define SPIWIFI_ACK   11   // a.k.a BUSY or READY pin
#define ESP32_GPIO0   -1

#define HEADER_STR "ISS (ZARYA)"
#define TLE_LEN 69
#define MAX_BUFFER 1024

#define SERVER "celestrak.org"
#define QUERY "/NORAD/elements/gp.php?CATNR=25544&FORMAT=TLE"

#define CHECK_DISPLAY_CONNECTION false
#define CHECK_COMPASS_CONNECTION false
