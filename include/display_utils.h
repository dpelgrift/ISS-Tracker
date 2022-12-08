#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define WIDTH 128
#define HEIGHT 64


Adafruit_SH1107 display = Adafruit_SH1107(HEIGHT, WIDTH, &Wire);

void resetDisplay(int16_t x, int16_t y, uint8_t textSize=1) {
    display.clearDisplay();
    display.display();
    display.setTextSize(textSize);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(x,y);
}

void clearDisplay() {
    display.clearDisplay();
    display.display();
}

int beginDisplay() {
    return display.begin(0x3C, true); // Address 0x3C default
}