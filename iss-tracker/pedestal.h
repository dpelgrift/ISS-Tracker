#pragma once
#include <Arduino.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_MMC56x3.h>
#include "coord.h"
#include "defs.h"

double steps2deg(long steps);
long deg2steps(double azDeg);

struct Pedestal {
    Servo servo;
    AccelStepper stepper;
    Adafruit_MMC5603 compass;
    sensors_event_t compassEvent;

    void begin();
    void zero();
    void setTargetAz(double azDeg);
    void setElevation(double el);
    void runStepper();
    void pointNorth();
    double getHeading();
    double getAverageHeading();
};
