#include <Arduino.h>
#include <string.h>
#include <SPI.h>
#include <Wire.h>
#include "arduino_secrets.h"
#include <coord.h>
#include <orbit_utils.h>
#include <wifi_utils.h>
#include <display_utils.h>
#include <pedestal.h>

#include "defs.h"


char ssid[] = SECRET_SSID;    // network SSID
char pass[] = SECRET_PASS;    // network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

// Objects/Wrappers
NtpQueryHandler ntp{};
TleQueryHandler tle{};
Pedestal ped{};

Orbit orb{};
Vec3 llaRef = {SECRET_LAT,SECRET_LON,0};
Vec3 posECI, velECI, posECEF, posLLA, posNED, posAER;
double era;
long lastUpdateTimeMillis;
long lastUnixUpdateMillis;

void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    delay(250);
    while (!Serial && WAIT_FOR_SERIAL) {}

    // Test calcBearing function
    double testBearing = calcBearing(llaRef[0],llaRef[1],MAG_NORTH_LAT,MAG_NORTH_LON);
    Serial.print("Bearing (deg): ");
    Serial.println(testBearing,3);

    delay(250); // wait for the OLED to power up
    /* Initialise the display */
    if (!display.begin(0x3C, true)) {  // Address 0x3C default
      /* There was a problem detecting the MMC5603 ... check your connections */
      Serial.println("No display detected");
      while (CHECK_DISPLAY_CONNECTION) delay(10);
    }

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    display.display();
    delay(1000);

    display.setRotation(1);
    resetDisplay(0,0,1);

    // Initialize pedestal controller
    ped.begin();

    // Get current pedestal azimuth
    double currAz = ped.getCompassHeading();
    Serial.printf("currAz (deg): %0.3f\n",currAz);
    ped.stepper.move(deg2steps(-currAz));
    while (ped.stepper.distanceToGo() != 0) {
        // long currPos = ped.stepper.currentPosition();
        // Serial.printf("Steps: %li, Deg: %0.3f\n",currPos,steps2deg(currPos));
        ped.stepper.run();
    }
    ped.stepper.setCurrentPosition(0);
    currAz = ped.getCompassHeading();
    Serial.printf("currAz (deg): %0.3f\n",currAz);

    // check for the WiFi module:
    WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
    while (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        // don't continue
        delay(1000);
    }

    String fv = WiFi.firmwareVersion();
    Serial.print("Found firmware "); Serial.println(fv);

    Serial.println("Scanning available networks...");
    listNetworks();

    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    display.print("Attempting to connect to SSID: ");
    display.println(ssid);
    display.display();

    // Connect to WPA/WPA2 network
    do {
        status = WiFi.begin(ssid, pass);
        delay(10000);     // wait until connection is ready!
    } while (status != WL_CONNECTED);

    Serial.println("Connected to wifi");
    display.println("Connected to wifi");
    display.display();
    delay(1000);

    // Start NTP connection
    ntp.begin();

    Serial.println("\nStarting connection to server...");
    display.println("\nStarting connection to server...");
    display.display();

    // Send TLE query
    tle.sendQuery();

    // Wait for response
    while (!tle.rcvData()){}
    tle.readTLE();

    Serial.println(tle.line1);
    Serial.println(tle.line2);

    // Parse received TLE
    orb = tle.getOrbit();

    Serial.println();
    Serial.print("epoch: "); Serial.println(orb.epoch_J);
    Serial.print("utc:   "); Serial.println(orb.epochUTC);
    Serial.print("incl:  "); Serial.println(orb.incl,8);
    Serial.print("a:     "); Serial.println(orb.a);
    Serial.print("ecc:   "); Serial.println(orb.ecc,8);
    Serial.print("Omega: "); Serial.println(orb.Omega,8);
    Serial.print("omega: "); Serial.println(orb.omega,8);
    Serial.print("M0:    "); Serial.println(orb.M0,8);
    Serial.print("n:     "); Serial.println(orb.n,8);
    Serial.print("n_dot: "); Serial.println(orb.n_dot,16);

    // Get unix time
    ntp.sendNTPpacket(); // send an NTP packet to a time server
    // wait to see if a reply is available
    delay(1000);
    ntp.parsePacket();

    lastUnixUpdateMillis = millis();
    lastUpdateTimeMillis = millis();
}

void loop() {

    // Advance stepper if necessary
    ped.runStepper();
    // long currPos = ped.stepper.currentPosition();
    // Serial.printf("Steps: %lu, Deg: %0.3f\n",currPos,steps2deg(currPos));

    if (millis() - lastUpdateTimeMillis > 1000) {
        long dt = (millis() - lastUnixUpdateMillis)/1000;

        // Calc ERA for current UTC
        era = getEraFromJulian(getJulianFromUnix(ntp.unixEpoch + dt));
        Serial.print("era:   "); Serial.println(era*RAD_TO_DEG,3);

        // Calc ECI Pos/Vel for current UTC
        orb.calcPosVelECI_UTC(ntp.unixEpoch + dt,posECI,velECI);
        posECEF = eci2ecef(posECI,-era);
        posLLA = ecef2lla(posECEF,DEGREES);
        posNED = ecef2ned(posECEF,llaRef,DEGREES);
        posAER = ned2AzElRng(posNED);

        // Update target position
        ped.setTargetAz(posAER[0]);

        Serial.printf("posECI:  [%0.3f,%0.3f,%0.3f]\n",posECI.x,posECI.y,posECI.z);
        Serial.printf("posECEF: [%0.3f,%0.3f,%0.3f]\n",posECEF.x,posECEF.y,posECEF.z);
        Serial.printf("posLLA:  [%0.3f,%0.3f,%0.3f]\n",posLLA.x,posLLA.y,posLLA.z);
        Serial.printf("posNED:  [%0.3f,%0.3f,%0.3f]\n",posNED.x,posNED.y,posNED.z);
        Serial.printf("posAER:  [%0.3f,%0.3f,%0.3f]\n",posAER.x,posAER.y,posAER.z);

        // Print status to display
        resetDisplay(0,10,1);
        display.printf("UTC: %lu\n",ntp.unixEpoch + dt);
        display.printf("posLLA: [%0.3f,%0.3f,%0.3f]\n",posLLA.x,posLLA.y,posLLA.z);
        display.printf("posAER: [%0.3f,%0.3f,%0.3f]\n",posAER.x,posAER.y,posAER.z);
        
        display.display();
        lastUpdateTimeMillis = millis();
    }
}


