#include "pedestal.h"

double steps2deg(long steps) {
    return double(steps) * 180.0 / STEPS_PER_REV;
}

long deg2steps(double azDeg) {
    return floor(azDeg * STEPS_PER_REV / 180.0);
}

void Pedestal::begin() {
    // Initialize Servo
    servo.attach(SERVO_PIN);
    servo.write(90);

    // Initialize stepper
    stepper = AccelStepper(AccelStepper::FULL4WIRE, STEP1, STEP2, STEP3, STEP4);
    stepper.setSpeed(STEPPER_SPEED);
    stepper.runToNewPosition(0);
    
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
    double currAz = getCurrPedestalAz();

    // Compute which direction is closer to current az
    double relAz = targetAzDeg - currAz;
    if (relAz > 180.0) {
        relAz -= 360.0;
    } else if (relAz < -180.0) {
        relAz += 360.0;
    }

    stepper.move(deg2steps(relAz));
}

void Pedestal::setElevation(double el) {
    servo.writeMicroseconds(map(long(el*100),0,18000,1000,2000));
}


void Pedestal::runStepper() {
    stepper.runSpeed();
}

double Pedestal::getCurrPedestalAz() {
    // Get compass heading
    compass.getEvent(&compassEvent);
    double heading = atan2(compassEvent.magnetic.y,compassEvent.magnetic.x) * RAD_TO_DEG;

    // Convert to true north heading
    heading += TRUE_NORTH_OFFSET_DEG;

    heading = fmod(heading + 360,360);
    return heading;
}