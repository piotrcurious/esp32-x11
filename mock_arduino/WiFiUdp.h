#ifndef WIFIUDP_H
#define WIFIUDP_H

#include <stdint.h>

class WiFiUDP {
public:
    int parsePacket() { return 0; }
    void read(uint8_t *buf, size_t size) {}
    void stop() {}
    void beginPacket(const char* host, uint16_t port) {}
    void write(const uint8_t *buf, size_t size) {}
    void endPacket() {}
    int available() { return 0; }
};

#endif
