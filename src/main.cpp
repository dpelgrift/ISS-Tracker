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

unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
// const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;

int read3LE(char* buff, char* line1, char* line2) {

    int idx = 0;
    String str = "";

    // Read lines until header found
    while (idx < MAX_BUFFER) {
        str += *buff; // Read character from buffer into string
        if (*buff == '\n') {
            if (str.startsWith(HEADER_STR)) break;
            else str = "";
        }
        buff++;
        idx++;
    }
    
    // If reached end of buffer, header not found
    if (idx == MAX_BUFFER) return -1;
    buff++;
    memcpy(line1, buff, 69);
    memcpy(line2, buff+71, 69);

    // Read line 1
    // while (*buff != '\n') {
    //     *line1 = *buff;
    //     line1++;
    // }
    // Read line 2
    // while (c != '\n') {
    //     c = client.read();
    //     *line2 = c;
    //     line2++;
    // }
    return 0;
}

void printWifiStatus() {
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your board's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}

void setup() {
    //Initialize serial and wait for port to open:
    Serial.begin(9600);
    while (!Serial) {}


    delay(250); // wait for the OLED to power up
    display.begin(0x3C, true); // Address 0x3C default

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

    // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    do {
        status = WiFi.begin(ssid, pass);
        delay(10000);     // wait until connection is ready!
    } while (status != WL_CONNECTED);

    Serial.println("Connected to wifi");
    // printWifiStatus();

    Serial.println("\nStarting connection to server...");
    // if you get a connection, report back via serial:
    sendQuery(client);

}

uint32_t bytes = 0;
char buff[MAX_BUFFER];
char line1[69], line2[69];

void loop() {
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
        char c = client.read();
        // Serial.write(c);
        buff[bytes]=c;
        bytes++;
    }



    // if the server's disconnected, stop the client:
    if (!client.connected()) {
        Serial.println();
        Serial.println("disconnecting from server.");
        client.stop();

        Serial.println(buff);

        // Parse read bytes into TLE lines
        read3LE(buff, line1, line2);

        Serial.print("Line 1: ");
        Serial.write(line1,69);
        Serial.print("\nLine 2: ");
        Serial.write(line2,69);

        Orbit orb{};
        orb.initFromTLE(line1,line2);

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
        Serial.print("n_dot: "); Serial.println(orb.n_dot,8);


        double era = getEraFromJulian(orb.epoch_J);
        Vec3 posECI = orb.calcPosECI(0);
        Vec3 posECEF = eci2ecef(posECI,era);
        Vec3 posLLA = ecef2lla(posECI,DEGREES);

        Serial.print("era:   "); Serial.println(era*RAD_TO_DEG,3);
        Serial.print("posECI: ["); 
        Serial.print(posECI.x,3); Serial.print(","); 
        Serial.print(posECI.y,3); Serial.print(","); 
        Serial.print(posECI.z,3); Serial.println("]"); 

        Serial.print("posECEF:["); 
        Serial.print(posECEF.x,3); Serial.print(","); 
        Serial.print(posECEF.y,3); Serial.print(","); 
        Serial.print(posECEF.z,3); Serial.println("]"); 

        Serial.print("posLLA: ["); 
        Serial.print(posLLA.x,3); Serial.print(","); 
        Serial.print(posLLA.y,3); Serial.print(","); 
        Serial.print(posLLA.z,3); Serial.println("]"); 


        Serial.println("\nStarting connection to NTP server...");
        Udp.begin(localPort);

        sendNTPpacket(Udp,timeServer,packetBuffer); // send an NTP packet to a time server
        // wait to see if a reply is available
        delay(1000);
        if (Udp.parsePacket()) {
            Serial.println("packet received");
            // We've received a packet, read the data from it
            Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

            //the timestamp starts at byte 40 of the received packet and is four bytes,
            // or two words, long. First, esxtract the two words:

            unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
            unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
            // combine the four bytes (two words) into a long integer
            // this is NTP time (seconds since Jan 1 1900):
            unsigned long secsSince1900 = highWord << 16 | lowWord;
            Serial.print("Seconds since Jan 1 1900 = ");
            Serial.println(secsSince1900);

            // now convert NTP time into everyday time:
            Serial.print("Unix time = ");
            // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
            const unsigned long seventyYears = 2208988800UL;
            // subtract seventy years:
            unsigned long epoch = secsSince1900 - seventyYears;
            // print Unix time:
            Serial.println(epoch);


            // print the hour, minute and second:
            Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
            Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
            Serial.print(':');
            if (((epoch % 3600) / 60) < 10) {
            // In the first 10 minutes of each hour, we'll want a leading '0'
            Serial.print('0');
            }
            Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
            Serial.print(':');
            if ((epoch % 60) < 10) {
            // In the first 10 seconds of each minute, we'll want a leading '0'
            Serial.print('0');
            }
            Serial.println(epoch % 60); // print the second
        }

        // do nothing forevermore:
        while (true);
    }
  }


