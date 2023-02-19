/*
  display_utils.h - Utility functions for handling Adafruit 128x64 OLED Featherwing: https://www.adafruit.com/product/4650
 */
#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "TimeLib.h"

#define WIDTH 128
#define HEIGHT 64

// Create display object
Adafruit_SH1107 display = Adafruit_SH1107(HEIGHT, WIDTH, &Wire);

// Clear display contents, set text size, and set cursor location
void resetDisplay(int16_t x, int16_t y, uint8_t textSize=1, bool doRefresh=false) {
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(x,y);

    if (doRefresh) display.display();
}

// Clear display contents and refresh
void clearDisplay() {
    display.clearDisplay();
    display.display();
}

// Start I2C connection to display
int beginDisplay() {
    return display.begin(0x3C, true); // Address 0x3C default
}

// Display current time and ISS Azimuth & Elevation
void displayCurrTime(double az, double el) {
    display.clearDisplay();

    // Display Time in "hh:mm" format
    display.setCursor(1,2);
    display.setTextSize(3);
    display.printf("%02i:%02i",hour(),minute());

    // Dividing Line
    display.writeFastHLine(0,28,WIDTH,SH110X_WHITE);

    // Display Date in "Www MM/dd" format
    display.setTextSize(1);
    display.setCursor(98,2);
    display.print(dayShortStr(weekday()));
    display.setCursor(98,16);
    display.printf("%02i/%02i",month(),day());

    // Display ISS Az/El
    display.setCursor(0,32);
    display.setTextSize(2);
    display.printf("Az:%03.1f\nEl:%03.1f",az,el);

    display.display();
}
