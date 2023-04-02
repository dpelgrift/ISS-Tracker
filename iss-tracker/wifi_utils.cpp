/*
  wifi_utils.cpp - Functions for sending and receiving TLE and NTP queries over Wifi
 */
#include "wifi_utils.h"

// Initialize UDP connection on local port
void NtpQueryHandler::begin() {
    Udp.begin(localPort);
}

// Send an NTP request to the time server at the given address
void NtpQueryHandler::sendNTPpacket() {
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;

    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket("time.nist.gov", 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

// Check if UDP Packet received and if so, parse it
bool NtpQueryHandler::parsePacket() {
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

        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        
        // subtract seventy years:
        unixEpoch = secsSince1900 - seventyYears;

        // Set system time based on received packet
        setTime(unixEpoch + timeZone * SECS_PER_HOUR);

        // print Unix time:
        Serial.println(int32_t(unixEpoch));
        lastQueryTimeMillis = millis();

        return true;
    }
    return false;
}

// Split full 3-Line-Element (3LE) string into Two-Line-Element components
// TLE Format: http://celestrak.org/columns/v04n03/#FAQ01
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
    memcpy(line1, buff, TLE_LEN);
    memcpy(line2, buff+71, TLE_LEN);

    return 0;
}

// Connect to Celestrak and send query for the latest ISS 3LE
void TleQueryHandler::sendQuery() {
    if (client.connect(SERVER, 80)) {
        Serial.println("connected to server");
        // Make a HTTP request:
        client.print("GET "); client.print(QUERY); client.println(" HTTP/1.1");

        client.println("Host: " SERVER);
        client.println("Connection: close");
        client.println();
    }
}

// Read received characters into a buffer and return true when we have received everything
bool TleQueryHandler::rcvData() {
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

// Parse characters in received buffer and store TLE lines
int TleQueryHandler::readTLE() {
    return read3LE(rcvBuffer,line1,line2);
}

// Create Orbit struct from parsed TLE strings
void TleQueryHandler::getOrbit(Orbit& orb) {
    orb.initFromTLE(line1,line2);
}

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

// List all visible Wifi Networks
// Note that the Adafruit WiFi Co-Processor can only connect to 2.4 GHz networks
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
