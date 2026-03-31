// created by bing AI
// X11client for esp32 Arduino c++ platform
// This code is not tested and may contain errors

#include "Xlib.h"
#include <WiFi.h>

// WiFi credentials
const char* ssid = "your-ssid";
const char* password = "your-password";

// X11 server address and port
const char* xhost = "127.0.0.1";
const int xport = 1; // Use DISPLAY :1

// X11 display and window variables
Display *display;
Window window;
int my_screen;
GC gc;
XFontStruct *font;

// Connect to WiFi network
void setupWiFi() {
  Serial.println("setupWiFi...");
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Connect to X11 server and create a window
void setupX11() {
  Serial.println("setupX11...");
  // Set the display name as host:port.screen
  char display_name[20];
  sprintf(display_name, "%s:%d.0", xhost, xport);

  Serial.println("Opening display...");
  // Open the display
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    Serial.println("Cannot connect to X server");
    return;
  }

  Serial.println("Display opened. Getting screen...");
  // Get the default screen and create a graphics context
  my_screen = DefaultScreen(display);

  Serial.println("Creating GC...");
  gc = DefaultGC(display, my_screen);

  Serial.println("Creating window...");
  // Create a simple window with black background and white border
  window = XCreateSimpleWindow(display, RootWindow(display, my_screen),
    10, 10, // x, y position
    200, 100, // width, height
    1, // border width
    WhitePixel(display, my_screen), // border color
    BlackPixel(display, my_screen)); // background color

  Serial.println("Storing name...");
  // Set the window title and select the events to handle
  XStoreName(display, window, "X11client for esp32");
  Serial.println("Selecting input...");
  XSelectInput(display, window, ExposureMask | KeyPressMask);

  Serial.println("Loading font...");
  // Load a fixed font and set it for the graphics context
  font = XLoadQueryFont(display, "fixed");
  if (font == NULL) {
    Serial.println("Cannot load font");
    return;
  }
  Serial.println("Setting font...");
  XSetFont(display, gc, font->fid);

  Serial.println("Mapping window...");
  // Map the window to the screen
  XMapWindow(display, window);
  Serial.println("setupX11 done.");
}

// Draw text on the window
void drawText(const char* text) {
  // Set the foreground color to white and draw the text
  XSetForeground(display, gc, WhitePixel(display, my_screen));
  XDrawString(display, window, gc, 50, 50, text, strlen(text));
}

// Handle X11 events
void handleX11Events() {
   XEvent event;

   if (display == NULL) return;

   // Check if there is an event in the queue
   if (XPending(display) > 0) {
     // Get the next event
     XNextEvent(display, &event);

     // Handle the event based on its type
     switch (event.type) {
       case Expose:
         // The window needs to be redrawn
         drawText("Hello world");
         break;
       case KeyPress:
         // A key was pressed
         Serial.println("Key pressed");
         break;
       default:
         // Ignore other events
         break;
     }
   }
}

void setup() {
   Serial.println("setup...");
   setupWiFi();
   setupX11();
}

void loop() {
   handleX11Events();
}
