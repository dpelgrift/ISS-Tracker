#include "pedestal.h"

double steps2deg(long steps) {
    return double(steps) * 360.0 / STEPS_PER_REV;
}

long deg2steps(double azDeg) {
    return floor(azDeg * STEPS_PER_REV / 360.0);
}

void Pedestal::begin() {
    // Initialize Servo
    servo.attach(SERVO_PIN);
    servo.write(90);

    // Initialize stepper
    stepper = AccelStepper(AccelStepper::FULL4WIRE, STEP1, STEP2, STEP3, STEP4);
    stepper.setMaxSpeed(STEPPER_SPEED);
    stepper.setAcceleration(STEPPER_ACCEL);
    // stepper.runToNewPosition(0);
    
    /* Initialise the compass */
    compass = Adafruit_MMC5603(12345);
    if (!compass.begin(MMC56X3_DEFAULT_ADDRESS, &Wire)) {  // I2C mode
        /* There was a problem detecting the MMC5603 ... check your connections */
        Serial.println("No compass detected");
        while (CHECK_COMPASS_CONNECTION) delay(10);
    }
}

void Pedestal::zero() {
    // Reset Az/El Position to zero
    servo.write(90);
    stepper.runToNewPosition(0);
}

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

void Pedestal::setElevation(double el) {
    servo.writeMicroseconds(map(long(el*100),0,18000,500,2500));
}

void Pedestal::runStepper() {
    stepper.run();
}

void Pedestal::pointNorth() {
    double currAz = getAverageHeading();
    Serial.printf("currAz (deg): %0.3f\n",currAz);
    stepper.move(deg2steps(-currAz));
    while (stepper.distanceToGo() != 0) {stepper.run();}
    currAz = getAverageHeading();
    stepper.move(deg2steps(-currAz));
    while (stepper.distanceToGo() != 0) {stepper.run();}
    
    currAz = getAverageHeading();
    Serial.printf("currAz (deg): %0.3f\n",currAz);
}

double Pedestal::getHeading() {
    // Get compass heading
    compass.getEvent(&compassEvent);
    return atan2(-compassEvent.magnetic.x,compassEvent.magnetic.y) * RAD_TO_DEG;
}

double Pedestal::getCurrPedestalAz() {
    // Get compass heading
    double heading = getHeading();

    // Convert to true north heading
    heading += TRUE_NORTH_OFFSET_DEG;

    heading = fmod(heading + 360,360);
    return heading;
}

double Pedestal::getAverageHeading() {
    // Compute average heading over a number of readings
    double heading = 0;
    for (size_t i = 0; i < 50; ++i) {
        heading += getHeading();
    }
    heading /= 50;
    heading += TRUE_NORTH_OFFSET_DEG;

    return heading;
}