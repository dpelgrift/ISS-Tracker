/*
  pedestal.cpp - Functions to control pedestal orientation and pointer elevation
 */
#include "pedestal.h"

// Convert # of steps in stepper motor to equivalent relative pedestal angle in degrees
double steps2deg(long steps) {
    return double(-steps) * 360.0 / STEPS_PER_REV;
}

// Convert pedestal angle in degrees to equivalent # of steps in stepper motor
long deg2steps(double azDeg) {
    return floor(-azDeg * STEPS_PER_REV / 360.0);
}

// Initialize Servo, Stepper, and Compass (if Active)
void Pedestal::begin() {
    // Initialize Servo
    servo.attach(SERVO_PIN);
    servo.write(90);

    // Initialize stepper
    stepper = AccelStepper(AccelStepper::FULL4WIRE, STEP1, STEP2, STEP3, STEP4);
    stepper.setMaxSpeed(STEPPER_SPEED);
    stepper.setAcceleration(STEPPER_ACCEL);
    
    // Initialise the compass
    compass = Adafruit_MMC5603(12345);
    if (!DO_BYPASS_COMPASS and !compass.begin(MMC56X3_DEFAULT_ADDRESS, &Wire)) {
        Serial.println("No compass detected");
        while (CHECK_COMPASS_CONNECTION) delay(10);
    }
}

// Set both pedestal azimuth and pointer elevation to zero
void Pedestal::zero() {
    // Reset Az/El Position to zero
    servo.write(90);
    stepper.runToNewPosition(0);
}

// Set target pedestal azimuth
void Pedestal::setTargetAz(double targetAzDeg) {
    double currAz = steps2deg(stepper.currentPosition());

    currAz = fmod(currAz + 3600,360.0);

    Serial.printf("currAz: %0.3f\n",currAz);

    // Compute which direction is closer to current az
    double relAz = targetAzDeg - currAz;
    if (relAz > 180.0) {
        relAz -= 360.0;
    } else if (relAz < -180.0) {
        relAz += 360.0;
    }

    Serial.printf("relAz: %0.3f\n",relAz);

    stepper.move(deg2steps(relAz));
}

// Set pointer elevation
void Pedestal::setElevation(double el) {
    // Convert double elevation to long hundredths of degrees to improve precision
    servo.writeMicroseconds(map(long(el*100),0,18000,SERVO_MIN_PWM,SERVO_MAX_PWM));
}

// Run stepper if needed
void Pedestal::runStepper() {
    stepper.run();
}

// Attempt to point pedestal northward based on average of multiple compass measurements
void Pedestal::pointNorth() {
    double currAz, relAz;
    currAz = getAverageHeading();
    Serial.printf("currAz (deg): %0.3f\n",currAz);
    relAz = -currAz;

    if (relAz > 180.0) {
        relAz -= 360.0;
    } else if (relAz < -180.0) {
        relAz += 360.0;
    }

    stepper.move(deg2steps(relAz));
    while (stepper.distanceToGo() != 0) {stepper.run();}
    currAz = getAverageHeading();
    Serial.printf("currAz (deg): %0.3f\n",currAz);
}

// Get reported compass heading in degrees from a single measurement
double Pedestal::getHeading() {
    // Get compass heading
    compass.getEvent(&compassEvent);
    return atan2(compassEvent.magnetic.x,-compassEvent.magnetic.y) * RAD_TO_DEG;
}


// Get average compass heading over multiple measurements
double Pedestal::getAverageHeading() {
    // Compute average heading over a number of samples
    const size_t nSamples=100;
    double heading = 0;
    for (size_t i = 0; i < nSamples; ++i) {
        heading += getHeading() / nSamples;
    }
    heading += TRUE_NORTH_OFFSET_DEG;
    heading = fmod(heading + 360,360);

    return heading;
}
