/*
  wifi_utils.h - Wrapper structs that handle sending and receiving TLE and NTP queries over Wifi
 */
#pragma once

#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "defs.h"
#include "orbit_utils.h"
#include "TimeLib.h"

#define NTP_PACKET_SIZE 48

const unsigned long seventyYears = 2208988800UL;
const unsigned int localPort = 2390;      // local port to listen for UDP packets

void printEncryptionType(int thisType);
void listNetworks();

int read3LE(char* buff, char* line1, char* line2);

// Struct to handle UDP querying of NTP Time Server
struct NtpQueryHandler {
    WiFiUDP Udp;
    byte packetBuffer[NTP_PACKET_SIZE];

    time_t unixEpoch;
    time_t lastQueryTimeMillis;

    void begin();
    void sendNTPpacket();
    bool parsePacket();
};

// Struct to handle HTTP connections and queries to Celestrak for ISS TLE data
struct TleQueryHandler {
    WiFiClient client;

    char rcvBuffer[MAX_BUFFER];
    uint32_t rcvBytes;

    char line1[TLE_LEN];
    char line2[TLE_LEN];

    void sendQuery();
    bool rcvData();
    int readTLE();
    void getOrbit(Orbit& orb);
};
