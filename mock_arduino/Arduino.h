#ifndef ARDUINO_H
#define ARDUINO_H

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdint>

#define WL_CONNECTED 1

inline void delay(int ms) {
    usleep(ms * 1000);
}

class SerialClass {
public:
    void begin(int baud) {}
    void print(const char* s) { std::cout << s; }
    void print(int i) { std::cout << (long)i; }
    void print(long i) { std::cout << i; }
    void println(const char* s = "") { std::cout << s << std::endl; }
    void println(int i) { std::cout << (long)i << std::endl; }
    void println(long i) { std::cout << i << std::endl; }
};

extern SerialClass Serial;

typedef uint8_t byte;

#endif
