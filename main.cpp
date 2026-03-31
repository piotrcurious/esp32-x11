#include "Xlib.h"
#include "mock_arduino/Arduino.h"
#include "mock_arduino/WiFi.h"
#include <iostream>
#include <fstream>

// Global objects for mock environment
SerialClass Serial;
WiFiClass WiFi;

// The global variables are defined in esp32_x11.ino
#include "esp32_x11.ino"

int main() {
    Serial.println("Starting main...");
    setup();
    Serial.println("setup() returned");

    // Run loop a few times to let the window map and expose
    for (int i = 0; i < 50; ++i) {
        loop();
        usleep(100000); // 100ms
    }

    // Take a screenshot of the window
    int w = 100;
    int h = 100;
    Serial.println("Taking screenshot of the window...");
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
        Serial.print("Running command: "); Serial.println(cmd);
        if (system(cmd) == 0) {
            Serial.println("Screenshot converted to screenshot.png");
        } else {
            Serial.println("Failed to convert screenshot to PNG");
        }
    } else {
        Serial.println("Failed to take screenshot");
    }

    XCloseDisplay(display);
    return 0;
}
