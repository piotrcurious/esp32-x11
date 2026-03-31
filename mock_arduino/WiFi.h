#ifndef WIFI_H
#define WIFI_H

#include "Arduino.h"

class WiFiClass {
public:
    void begin(const char* ssid, const char* password) {}
    int status() { return WL_CONNECTED; }
    const char* localIP() { return "127.0.0.1"; }
};

extern WiFiClass WiFi;

#endif
