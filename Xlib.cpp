// Xlib.cpp
#include "Xlib.h"
#include "mock_arduino/WiFiClient.h"
#include <iostream>

#define BUFSIZE 2048
#define X_PROTOCOL 11

// Protocol Opcodes
#define X_CreateWindow 1
#define X_ChangeWindowAttributes 2
#define X_MapWindow 8
#define X_ChangeProperty 18
#define X_OpenFont 45
#define X_QueryFont 47
#define X_CreateGC 55
#define X_ChangeGC 56
#define X_PolyText8 74
#define X_CloseDisplay 42

static XID _XAllocID(Display *dpy) {
    XID id = dpy->resource_id;
    dpy->resource_id++;
    return id;
}

extern "C" {

Display *XOpenDisplay(const char *display_name) {
    Display *dpy = (Display *)malloc(sizeof(Display));
    if (!dpy) return NULL;
    memset(dpy, 0, sizeof(Display));
    dpy->client = new WiFiClient();

    char host[128];
    int port_num = 0;
    int screen_num = 0;

    const char *colon = strchr(display_name, ':');
    if (!colon) {
        delete dpy->client;
        free(dpy);
        return NULL;
    }
    int host_len = colon - display_name;
    if (host_len >= 128) host_len = 127;
    strncpy(host, display_name, host_len);
    host[host_len] = '\0';
    if (host[0] == '\0') strcpy(host, "localhost");

    port_num = atoi(colon + 1);
    const char *dot = strchr(colon, '.');
    if (dot) screen_num = atoi(dot + 1);

    std::cout << "Connecting to " << host << ":" << (6000 + port_num) << std::endl;
    if (!dpy->client->connect(host, 6000 + port_num)) {
        std::cout << "Connection failed" << std::endl;
        delete dpy->client;
        free(dpy);
        return NULL;
    }

    dpy->buffer = (char *)malloc(BUFSIZE);
    if (!dpy->buffer) {
        delete dpy->client;
        free(dpy);
        return NULL;
    }
    dpy->bufptr = dpy->buffer;
    dpy->buffer_size = BUFSIZE;

    // Send Connection Setup
    uint8_t setup[12] = {0};
    setup[0] = 'l'; // Little endian
    setup[2] = X_PROTOCOL; // Major
    setup[4] = 0; // Minor
    dpy->client->write(setup, 12);

    // Read Response
    uint8_t resp[8];
    if (dpy->client->readBytes((char*)resp, 8) != 8) {
        std::cout << "Failed to read setup response" << std::endl;
        delete dpy->client;
        free(dpy->buffer);
        free(dpy);
        return NULL;
    }

    if (resp[0] != 1) {
        std::cout << "Connection rejected: " << (int)resp[0] << std::endl;
        delete dpy->client;
        free(dpy->buffer);
        free(dpy);
        return NULL;
    }

    uint16_t length = *(uint16_t*)(resp + 6);
    int full_len = length * 4;
    uint8_t *data = (uint8_t*)malloc(full_len);
    if (!data) {
        delete dpy->client;
        free(dpy->buffer);
        free(dpy);
        return NULL;
    }
    dpy->client->readBytes((char*)data, full_len);

    dpy->resource_base = *(uint32_t*)(data + 4);
    dpy->resource_mask = *(uint32_t*)(data + 8);
    dpy->resource_id = dpy->resource_base;

    uint16_t vendor_len = *(uint16_t*)(data + 16);
    uint8_t num_screens = data[24];
    uint8_t num_formats = data[25];

    uint8_t *p = data + 32;
    p += ((vendor_len + 3) & ~3); // skip vendor
    p += (num_formats * 8); // skip formats

    dpy->nscreens = num_screens;
    dpy->screens = (Screen *)malloc(sizeof(Screen) * dpy->nscreens);
    if (!dpy->screens) {
        free(data);
        delete dpy->client;
        free(dpy->buffer);
        free(dpy);
        return NULL;
    }

    for (int i = 0; i < dpy->nscreens; i++) {
        dpy->screens[i].root = *(uint32_t*)p;
        dpy->screens[i].white_pixel = *(uint32_t*)(p + 12);
        dpy->screens[i].black_pixel = *(uint32_t*)(p + 16);
        dpy->screens[i].depth = p[38];

        uint8_t n_depths = p[39];
        p += 40;
        for (int d = 0; d < n_depths; d++) {
            uint16_t n_visuals = *(uint16_t*)(p + 2);
            p += 8 + n_visuals * 24;
        }
    }

    dpy->default_screen_no = screen_num;
    free(data);
    std::cout << "X11 Display opened successfully." << std::endl;
    return dpy;
}

Window XCreateSimpleWindow(Display *dpy, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background) {
    Window w = _XAllocID(dpy);
    uint32_t req[8 + 2];
    req[0] = (X_CreateWindow) | (10 << 16);
    req[1] = (uint32_t)w;
    req[2] = (uint32_t)parent;
    req[3] = (x & 0xFFFF) | (y << 16);
    req[4] = (width & 0xFFFF) | (height << 16);
    req[5] = (border_width & 0xFFFF) | (1 << 16); // InputOutput
    req[6] = 0; // Visual
    req[7] = 0x03; // background-pixel | border-pixel
    req[8] = (uint32_t)background;
    req[9] = (uint32_t)border;
    dpy->client->write((uint8_t *)req, 40);
    return w;
}

int XMapWindow(Display *dpy, Window w) {
    uint32_t req[2];
    req[0] = X_MapWindow | (2 << 16);
    req[1] = (uint32_t)w;
    dpy->client->write((uint8_t *)req, 8);
    return 1;
}

int XStoreName(Display *dpy, Window w, const char *name) {
    int len = strlen(name);
    int pad = (4 - (len & 3)) & 3;
    int req_len = 6 + (len + pad) / 4;
    uint32_t *req = (uint32_t *)calloc(req_len, 4);
    req[0] = X_ChangeProperty | (req_len << 16);
    req[1] = (uint32_t)w;
    req[2] = 39; // WM_NAME
    req[3] = 31; // STRING
    req[4] = 8;  // format
    req[5] = (uint32_t)len;
    memcpy(&req[6], name, len);
    dpy->client->write((uint8_t *)req, req_len * 4);
    free(req);
    return 1;
}

int XSelectInput(Display *dpy, Window w, long event_mask) {
    uint32_t req[4];
    req[0] = X_ChangeWindowAttributes | (4 << 16);
    req[1] = (uint32_t)w;
    req[2] = 0x800; // event-mask
    req[3] = (uint32_t)event_mask;
    dpy->client->write((uint8_t *)req, 16);
    return 1;
}

XFontStruct *XLoadQueryFont(Display *dpy, const char *name) {
    Font fid = _XAllocID(dpy);
    int len = strlen(name);
    int pad = (4 - (len & 3)) & 3;
    int req_len = 3 + (len + pad) / 4;
    uint32_t *req = (uint32_t *)calloc(req_len, 4);
    req[0] = X_OpenFont | (req_len << 16);
    req[1] = (uint32_t)fid;
    req[2] = (uint32_t)len;
    memcpy(&req[3], name, len);
    dpy->client->write((uint8_t *)req, req_len * 4);
    free(req);

    XFontStruct *fs = (XFontStruct *)malloc(sizeof(XFontStruct));
    fs->fid = fid;
    fs->ascent = 12;
    fs->descent = 4;
    return fs;
}

int XDrawString(Display *dpy, Drawable d, GC gc, int x, int y, const char *string, int length) {
    int pad = (4 - (length & 3)) & 3;
    int req_len = 4 + (length + pad) / 4;
    uint32_t *req = (uint32_t *)calloc(req_len, 4);
    req[0] = X_PolyText8 | (req_len << 16);
    req[1] = (uint32_t)d;
    req[2] = (uint32_t)(uintptr_t)gc;
    req[3] = (x & 0xFFFF) | (y << 16);
    uint8_t *p = (uint8_t*)&req[4];
    *p++ = (uint8_t)length;
    *p++ = 0; // delta
    memcpy(p, string, length);
    dpy->client->write((uint8_t *)req, req_len * 4);
    free(req);
    return 1;
}

int XSetForeground(Display *dpy, GC gc, unsigned long foreground) {
    uint32_t req[4];
    req[0] = X_ChangeGC | (4 << 16);
    req[1] = (uint32_t)(uintptr_t)gc;
    req[2] = 0x04; // foreground
    req[3] = (uint32_t)foreground;
    // Mock CreateGC if gc is 1
    static bool gc_initialized = false;
    if (!gc_initialized) {
        uint32_t creq[4];
        creq[0] = X_CreateGC | (4 << 16);
        creq[1] = (uint32_t)(uintptr_t)gc;
        creq[2] = dpy->screens[0].root;
        creq[3] = 0;
        dpy->client->write((uint8_t *)creq, 16);
        gc_initialized = true;
    }
    dpy->client->write((uint8_t *)req, 16);
    return 1;
}

int XSetFont(Display *dpy, GC gc, Font font) {
    uint32_t req[4];
    req[0] = X_ChangeGC | (4 << 16);
    req[1] = (uint32_t)(uintptr_t)gc;
    req[2] = 0x4000; // font
    req[3] = (uint32_t)font;
    dpy->client->write((uint8_t *)req, 16);
    return 1;
}

int XPending(Display *dpy) {
    return dpy->client->available();
}

int XNextEvent(Display *dpy, XEvent *event) {
    uint8_t buf[32];
    if (dpy->client->readBytes((char*)buf, 32) != 32) return 0;
    event->type = buf[0] & 0x7F;
    if (event->type == Expose) {
        event->xexpose.window = *(uint32_t*)(buf + 4);
    }
    return 1;
}

int XGetWindowAttributes(Display *dpy, Window w, XWindowAttributes *wa) {
    wa->width = 200; wa->height = 100;
    return 1;
}

int XTextWidth(XFontStruct *fs, const char *s, int len) {
    return len * 8;
}

int XCloseDisplay(Display *dpy) {
    if (!dpy) return 0;
    uint32_t req = X_CloseDisplay | (1 << 16);
    if (dpy->client) {
        dpy->client->write((uint8_t*)&req, 4);
        delete dpy->client;
    }
    if (dpy->screens) free(dpy->screens);
    if (dpy->buffer) free(dpy->buffer);
    free(dpy);
    return 1;
}

int XFlush(Display *dpy) { return 1; }

} // extern "C"
