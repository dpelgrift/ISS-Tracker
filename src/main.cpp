#include <Arduino.h>
#include <string.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_MMC56x3.h>
#include "arduino_secrets.h"
#include <coord.h>
#include <orbit_utils.h>
#include <wifi_utils.h>

#include "defs.h"

Adafruit_MMC5603 mag = Adafruit_MMC5603(12345);
Adafruit_SH1107 display = Adafruit_SH1107(64, 128, &Wire);


char ssid[] = SECRET_SSID;    // network SSID
char pass[] = SECRET_PASS;    // network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;




NtpQueryHandler ntp{};
TleQueryHandler tle{};


void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      Serial.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      Serial.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      Serial.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      Serial.println("None");
      break;
    case ENC_TYPE_AUTO:
      Serial.println("Auto");
      break;
    case ENC_TYPE_UNKNOWN:
    default:
      Serial.println("Unknown");
      break;
  }
}


void printMacAddress(byte mac[]) {
  for (int i = 5; i >= 0; i--) {
    if (mac[i] < 16) {
      Serial.print("0");
    }
    Serial.print(mac[i], HEX);
    if (i > 0) {
      Serial.print(":");
    }
  }
  Serial.println();
}

void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't get a wifi connection");
    while (true);
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
  }
}


void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {}


    delay(250); // wait for the OLED to power up
    display.begin(0x3C, true); // Address 0x3C default

    // Show image buffer on the display hardware.
    // Since the buffer is intialized with an Adafruit splashscreen
    // internally, this will display the splashscreen.
    display.display();
    delay(1000);

    display.clearDisplay();
    display.display();
    display.setRotation(1);
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0,0);

    /* Initialise the compass */
    // if (!mag.begin(MMC56X3_DEFAULT_ADDRESS, &Wire)) {  // I2C mode
    //   /* There was a problem detecting the MMC5603 ... check your connections */
    //   Serial.println("Ooops, no MMC5603 detected ... Check your wiring!");
    //   while (1) delay(10);
    // }

    // check for the WiFi module:
    WiFi.setPins(SPIWIFI_SS, SPIWIFI_ACK, ESP32_RESETN, ESP32_GPIO0, &SPIWIFI);
    while (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        // don't continue
        delay(1000);
    }

    String fv = WiFi.firmwareVersion();
    if (fv < "1.0.0") {
        Serial.println("Please upgrade the firmware");
    }
    Serial.print("Found firmware "); Serial.println(fv);

    Serial.println("Scanning available networks...");
    listNetworks();

    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    display.print("Attempting to connect to SSID: ");
    display.println(ssid);
    display.display();
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    do {
        status = WiFi.begin(ssid, pass);
        delay(10000);     // wait until connection is ready!
    } while (status != WL_CONNECTED);

    Serial.println("Connected to wifi");
    display.println("Connected to wifi");
    display.display();
    delay(1000);
    // printWifiStatus();

    ntp.begin();

    Serial.println("\nStarting connection to server...");
    display.println("\nStarting connection to server...");
    display.display();
    // if you get a connection, report back via serial:
    tle.sendQuery();

}

void loop() {

    if (tle.rcvData()) {
        tle.real3LE();

        Orbit orb = tle.getOrbit();

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


        double era = getEraFromJulian(orb.epoch_J);
        Serial.print("era:   "); Serial.println(era*RAD_TO_DEG,3);

        Vec3 posECI, velECI;
        orb.calcPosVelECI(0,posECI,velECI);

        Serial.print("posECI: ["); 
        Serial.print(posECI.x,3); Serial.print(","); 
        Serial.print(posECI.y,3); Serial.print(","); 
        Serial.print(posECI.z,3); Serial.println("]"); 

        Serial.print("velECI: ["); 
        Serial.print(velECI.x,3); Serial.print(","); 
        Serial.print(velECI.y,3); Serial.print(","); 
        Serial.print(velECI.z,3); Serial.println("]"); 

        Vec3 posECEF = eci2ecef(posECI,-era);
        Serial.print("posECEF: ["); 
        Serial.print(posECEF.x,3); Serial.print(","); 
        Serial.print(posECEF.y,3); Serial.print(","); 
        Serial.print(posECEF.z,3); Serial.println("]"); 

        Vec3 posLLA = ecef2lla(posECEF,DEGREES);
        Serial.print("posLLA: ["); 
        Serial.print(posLLA.x,3); Serial.print(","); 
        Serial.print(posLLA.y,3); Serial.print(","); 
        Serial.print(posLLA.z,3); Serial.println("]"); 

        Vec3 llaRef = {SECRET_LAT,SECRET_LON,0};
        Vec3 posNED = ecef2ned(posECEF,llaRef,DEGREES);
        Serial.print("posNED: ["); 
        Serial.print(posNED.x,3); Serial.print(","); 
        Serial.print(posNED.y,3); Serial.print(","); 
        Serial.print(posNED.z,3); Serial.println("]"); 

        Vec3 posAER = ned2AzElRng(posNED);
        Serial.print("posAER: ["); 
        Serial.print(posAER.x,3); Serial.print(","); 
        Serial.print(posAER.y,3); Serial.print(","); 
        Serial.print(posAER.z,3); Serial.println("]"); 

        while (true) {
            ntp.sendNTPpacket(); // send an NTP packet to a time server
            // wait to see if a reply is available
            delay(1000);
            ntp.parsePacket();

            // Calc ERA for current UTC
            era = getEraFromJulian(getJulianFromUnix(ntp.unixEpoch));
            // Calc ECI Pos/Vel for current UTC
            orb.calcPosVelECI_UTC(ntp.unixEpoch,posECI,velECI);

            Serial.print("era:   "); Serial.println(era*RAD_TO_DEG,3);

            Serial.print("posECI: ["); 
            Serial.print(posECI.x,3); Serial.print(","); 
            Serial.print(posECI.y,3); Serial.print(","); 
            Serial.print(posECI.z,3); Serial.println("]"); 

            posECEF = eci2ecef(posECI,-era);
            Serial.print("posECEF: ["); 
            Serial.print(posECEF.x,3); Serial.print(","); 
            Serial.print(posECEF.y,3); Serial.print(","); 
            Serial.print(posECEF.z,3); Serial.println("]"); 

            posLLA = ecef2lla(posECEF,DEGREES);
            Serial.print("posLLA: ["); 
            Serial.print(posLLA.x,3); Serial.print(","); 
            Serial.print(posLLA.y,3); Serial.print(","); 
            Serial.print(posLLA.z,3); Serial.println("]"); 

            posNED = ecef2ned(posECEF,llaRef,DEGREES);

            Serial.print("posNED: ["); 
            Serial.print(posNED.x,3); Serial.print(","); 
            Serial.print(posNED.y,3); Serial.print(","); 
            Serial.print(posNED.z,3); Serial.println("]");

            posAER = ned2AzElRng(posNED);
            Serial.print("posAER: ["); 
            Serial.print(posAER.x,3); Serial.print(","); 
            Serial.print(posAER.y,3); Serial.print(","); 
            Serial.print(posAER.z,3); Serial.println("]"); 

            delay(10000);
        };
    }
}


