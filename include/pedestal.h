#pragma once
#include <Arduino.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_MMC56x3.h>
#include <coord.h>
#include <defs.h>

#define STEPS_PER_REV (2038*4)
#define STEPPER_SPEED 600
#define STEPPER_ACCEL 300

// Stepper pins
#define STEP1 A1
#define STEP2 A3
#define STEP3 A2
#define STEP4 A4

#define SERVO_PIN 10

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
    double getCurrPedestalAz();
    double getAverageHeading();
};