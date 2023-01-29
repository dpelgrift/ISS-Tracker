#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "TimeLib.h"

#define WIDTH 128
#define HEIGHT 64


Adafruit_SH1107 display = Adafruit_SH1107(HEIGHT, WIDTH, &Wire);

void resetDisplay(int16_t x, int16_t y, uint8_t textSize=1, bool doRefresh=false) {
    display.clearDisplay();
    display.setTextSize(textSize);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(x,y);

    if (doRefresh) display.display();
}

void clearDisplay() {
    display.clearDisplay();
    display.display();
}

int beginDisplay() {
    return display.begin(0x3C, true); // Address 0x3C default
}

void displayCurrTime(double az, double el) {
    display.clearDisplay();

    // // Display Time in "hh:mm" format
    // display.setCursor(4,2);
    // display.setTextSize(4);
    // display.printf("%02i:%02i",hour(),minute());

    // // Dividing Line
    // display.writeFastHLine(0,38,WIDTH,SH110X_WHITE);

    // // Display Date in "Www MM/dd" format
    // display.setCursor(2,46);
    // display.setTextSize(2);
    // display.print(dayShortStr(weekday()));
    // display.setCursor(68,46);
    // display.printf("%02i/%02i",month(),day());

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