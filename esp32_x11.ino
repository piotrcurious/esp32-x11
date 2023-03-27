// created by bing AI
// X11client for esp32 Arduino c++ platform
// This code is not tested and may contain errors

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <WiFi.h>

// WiFi credentials
const char* ssid = "your-ssid";
const char* password = "your-password";

// X11 server address and port
const char* xhost = "your-xhost";
const int xport = 6000;

// X11 display and window variables
Display *display;
Window window;
int screen;
GC gc;
XFontStruct *font;

// Connect to WiFi network
void setupWiFi() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

// Connect to X11 server and create a window
void setupX11() {
  // Set the display name as host:port
  char display_name[20];
  sprintf(display_name, "%s:%d", xhost, xport);

  // Open the display
  display = XOpenDisplay(display_name);
  if (display == NULL) {
    Serial.println("Cannot connect to X server");
    return;
  }

  // Get the default screen and create a graphics context
  screen = DefaultScreen(display);
  gc = DefaultGC(display, screen);

  // Create a simple window with black background and white border
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 
    10, 10, // x, y position
    200, 100, // width, height
    1, // border width
    WhitePixel(display, screen), // border color
    BlackPixel(display, screen)); // background color

  // Set the window title and select the events to handle
  XStoreName(display, window, "X11client for esp32");
  XSelectInput(display, window, ExposureMask | KeyPressMask);

  // Load a fixed font and set it for the graphics context
  font = XLoadQueryFont(display, "fixed");
  if (font == NULL) {
    Serial.println("Cannot load font");
    return;
  }
  XSetFont(display, gc, font->fid);

  // Map the window to the screen
  XMapWindow(display, window);
}

// Draw text on the window
void drawText(const char* text) {
  // Get the text width and height
  int text_width = XTextWidth(font, text, strlen(text));
  int text_height = font->ascent + font->descent;

  // Get the window width and height
  XWindowAttributes wa;
  XGetWindowAttributes(display, window, &wa);
  int win_width = wa.width;
  int win_height = wa.height;

  // Calculate the text position to center it on the window
  int x = (win_width - text_width) / 2;
  int y = (win_height - text_height) / 2 + font->ascent;

  // Set the foreground color to white and draw the text
  XSetForeground(display, gc, WhitePixel(display, screen));
  XDrawString(display, window, gc, x, y, text, strlen(text));
}

// Handle X11 events
void handleX11Events() {
   XEvent event;

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
   setupWiFi();
   setupX11();
}

void loop() {
   handleX11Events();
}