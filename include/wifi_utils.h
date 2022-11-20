#include <Arduino.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include "defs.h"

// send an NTP request to the time server at the given address
void sendNTPpacket(WiFiUDP& Udp, IPAddress& address, byte* packetBuffer) {
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
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}


void sendQuery(WiFiClient& client) {
    if (client.connect(SERVER, 80)) {
        Serial.println("connected to server");
        // Make a HTTP request:
        client.print("GET "); client.print(QUERY); client.println(" HTTP/1.1");

        client.println("Host: " SERVER);
        client.println("Connection: close");
        client.println();
    }
}