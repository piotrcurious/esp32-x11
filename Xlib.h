#ifndef XLIB_H
#define XLIB_H

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>

// Forward declarations for the internal types
#ifdef __cplusplus
class WiFiClient;
class WiFiUDP;
#endif

// X11 Protocol Basic Types
typedef uint32_t XID;
typedef XID Window;
typedef XID Drawable;
typedef XID Font;
typedef XID Colormap;
typedef XID GContext;
typedef XID Atom;
typedef uint32_t VisualID;
typedef uint32_t Time;
typedef GContext GC;
typedef bool Bool;

#define None 0L
#define True true
#define False false

// Event masks
#define NoEventMask 0L
#define KeyPressMask (1L<<0)
#define ExposureMask (1L<<15)

// Event types
#define KeyPress 2
#define Expose 12

#ifdef __cplusplus
extern "C" {
#endif

struct _XDisplay;
typedef struct _XDisplay Display;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
} XAnyEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    int x, y;
    int width, height;
    int count;
} XExposeEvent;

typedef struct {
    int type;
    unsigned long serial;
    Bool send_event;
    Display *display;
    Window window;
    Window root;
    Window subwindow;
    Time time;
    int x, y;
    int x_root, y_root;
    unsigned int state;
    unsigned int keycode;
    Bool same_screen;
} XKeyEvent;

typedef union _XEvent {
    int type;
    XAnyEvent xany;
    XExposeEvent xexpose;
    XKeyEvent xkey;
    char pad[96];
} XEvent;

typedef struct {
    int ascent;
    int descent;
    Font fid;
} XFontStruct;

typedef struct {
    int width;
    int height;
} XWindowAttributes;

typedef struct {
    Window root;
    unsigned long white_pixel;
    unsigned long black_pixel;
    int depth;
} Screen;

#ifdef __cplusplus
}
#endif

// The actual Display structure (exposed for the mock implementation)
struct _XDisplay {
    WiFiClient* client;
    WiFiUDP* udp;
    char *display_name;
    int default_screen_no;
    Screen *screens;
    int nscreens;
    uint32_t resource_id;
    uint32_t resource_base;
    uint32_t resource_mask;
    char *buffer;
    char *bufptr;
    int buffer_size;
};

#ifdef __cplusplus
extern "C" {
#endif

// Function Prototypes
Display *XOpenDisplay(const char *display_name);
int XCloseDisplay(Display *display);
int XFlush(Display *display);
int XPending(Display *display);
int XNextEvent(Display *display, XEvent *event);

Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background);
int XMapWindow(Display *display, Window w);
int XStoreName(Display *display, Window w, const char *name);
int XSelectInput(Display *display, Window w, long event_mask);

XFontStruct *XLoadQueryFont(Display *display, const char *name);
int XDrawString(Display *display, Drawable d, GC gc, int x, int y, const char *string, int length);
int XSetForeground(Display *display, GC gc, unsigned long foreground);
int XSetFont(Display *display, GC gc, Font font);

int XGetWindowAttributes(Display *display, Window w, XWindowAttributes *window_attributes_return);
int XTextWidth(XFontStruct *font_struct, const char *string, int count);

// Macros for compatibility
#define DefaultScreen(dpy) ((dpy)->default_screen_no)
#define RootWindow(dpy, scr) ((dpy)->screens[scr].root)
#define WhitePixel(dpy, scr) ((dpy)->screens[scr].white_pixel)
#define BlackPixel(dpy, scr) ((dpy)->screens[scr].black_pixel)
#define DefaultGC(dpy, scr) ((GC)1)

#ifdef __cplusplus
}
#endif

#endif
