#include "Xlib.h"
#include "mock_arduino/Arduino.h"
#include "mock_arduino/WiFi.h"
#include <iostream>
#include <fstream>

// Global objects for mock environment
SerialClass Serial;
WiFiClass WiFi;

#include "esp32_x11.ino"

int main() {
    Serial.println("Starting main...");
    setup();
    Serial.println("setup() returned");

    // Run loop many times
    for (int i = 0; i < 100; ++i) {
        loop();
        usleep(10000); // 10ms
    }

    // Take a screenshot of the window
    int w = 200;
    int h = 100;
    Serial.println("Taking screenshot of the created window...");
    XImage *img = XGetImage(display, window, 0, 0, w, h, 0xFFFFFFFF, ZPixmap);
    if (img) {
        Serial.println("Screenshot taken, saving to screenshot.raw");
        std::ofstream f("screenshot.raw", std::ios::binary);
        f.write(img->data, w * h * 4);
        f.close();
        XDestroyImage(img);

        // Convert to PNG using the python script
        char cmd[256];
        sprintf(cmd, "python3 make_png.py %d %d screenshot.raw screenshot.png", w, h);
        if (system(cmd) == 0) {
            Serial.println("Screenshot converted to screenshot.png");
        }
    } else {
        Serial.println("Failed to take screenshot");
    }

    XCloseDisplay(display);
    return 0;
}
