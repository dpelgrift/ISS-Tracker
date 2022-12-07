#pragma once

#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "defs.h"

const unsigned long seventyYears = 2208988800UL;
const unsigned int localPort = 2390;      // local port to listen for UDP packets


struct NtpQueryHandler {
    WiFiUDP Udp;
    IPAddress timeserver;
    byte packetBuffer[NTP_PACKET_SIZE];

    unsigned long unixEpoch;
    unsigned long lastQueryTimeMillis;

    NtpQueryHandler() {
        timeserver = IPAddress(129, 6, 15, 28);
    }

    inline void begin() {
        Udp.begin(localPort);
    }

    // send an NTP request to the time server at the given address
    inline void sendNTPpacket() {
        //Serial.println("1");
        // set all bytes in the buffer to 0
        memset(packetBuffer, 0, NTP_PACKET_SIZE);
        // Initialize values needed to form NTP request
        // (see URL above for details on the packets)
        //Serial.println("2");
        packetBuffer[0] = 0b11100011;   // LI, Version, Mode
        packetBuffer[1] = 0;     // Stratum, or type of clock
        packetBuffer[2] = 6;     // Polling Interval
        packetBuffer[3] = 0xEC;  // Peer Clock Precision
        // 8 bytes of zero for Root Delay & Root Dispersion
        packetBuffer[12]  = 49;
        packetBuffer[13]  = 0x4E;
        packetBuffer[14]  = 49;
        packetBuffer[15]  = 52;

        //Serial.println("3");

        // all NTP fields have been given values, now
        // you can send a packet requesting a timestamp:
        Udp.beginPacket(timeserver, 123); //NTP requests are to port 123
        //Serial.println("4");
        Udp.write(packetBuffer, NTP_PACKET_SIZE);
        //Serial.println("5");
        Udp.endPacket();
        //Serial.println("6");
    }

    inline bool parsePacket() {
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
            
            // subtract seventy years:
            unixEpoch = secsSince1900 - seventyYears;
            // print Unix time:
            Serial.println(unixEpoch);
            lastQueryTimeMillis = millis();

            return true;
        }
        return false;
    }
};

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

    return 0;
}

struct TleQueryHandler {
    WiFiClient client;

    char rcvBuffer[MAX_BUFFER];
    uint32_t rcvBytes;

    char line1[69];
    char line2[69];

    inline void sendQuery() {
        if (client.connect(SERVER, 80)) {
            Serial.println("connected to server");
            // Make a HTTP request:
            client.print("GET "); client.print(QUERY); client.println(" HTTP/1.1");

            client.println("Host: " SERVER);
            client.println("Connection: close");
            client.println();
        }
    }

    inline bool rcvData() {
        if (!client.connected()){
            Serial.println();
            Serial.println("disconnecting from server.");
            client.stop();
            rcvBytes = 0;
            return true; 
        }
        while (client.available()) {
            char c = client.read();
            rcvBuffer[rcvBytes]=c;
            rcvBytes++;
        }
        return false;
    }
    
    inline int real3LE() {
        return read3LE(rcvBuffer,line1,line2);
    }

    inline Orbit getOrbit() {
        Orbit orb{};
        orb.initFromTLE(line1,line2);
        return orb;
    }
};

