#pragma once

#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "defs.h"
#include "orbit_utils.h"


#define NTP_PACKET_SIZE 48

const unsigned long seventyYears = 2208988800UL;
const unsigned int localPort = 2390;      // local port to listen for UDP packets

void printEncryptionType(int thisType);

void listNetworks();

int read3LE(char* buff, char* line1, char* line2);


struct NtpQueryHandler {
    WiFiUDP Udp;
    IPAddress timeserver;
    byte packetBuffer[NTP_PACKET_SIZE];

    unsigned long unixEpoch;
    unsigned long lastQueryTimeMillis;

    NtpQueryHandler();

    void begin();

    // send an NTP request to the time server at the given address
    void sendNTPpacket();

    bool parsePacket();
};

struct TleQueryHandler {
    WiFiClient client;

    char rcvBuffer[MAX_BUFFER];
    uint32_t rcvBytes;

    char line1[TLE_LEN];
    char line2[TLE_LEN];

    void sendQuery();
    bool rcvData();
    int readTLE();
    Orbit getOrbit();
};
