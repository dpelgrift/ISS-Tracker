#pragma once
#include <Arduino.h>
#include <AccelStepper.h>
#include <Servo.h>
#include <Wire.h>
#include <Adafruit_MMC56x3.h>
#include <coord.h>
#include <defs.h>

#define STEPS_PER_REV 2038*4
#define STEPPER_SPEED 400

// Stepper pins
#define STEP1 PIN_A0
#define STEP2 PIN_A1
#define STEP3 PIN_A2
#define STEP4 PIN_A3

#define SERVO_PIN 8

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
    void setCurrAz(long steps);
    double getCurrPedestalAz();
};