// Dreamed by Bing AI
// Xlib.h implementation for Esp32 Arduino platform
// This code is not tested and may contain errors

#ifndef XLIB_H
#define XLIB_H

#include <WiFiClient.h>
#include <WiFiUdp.h>

// X11 protocol constants and types
#define X_PROTOCOL 11
#define X_PROTOCOL_REVISION 0
#define sz_xConnClientPrefix 12
#define sz_xConnSetupPrefix 8
#define sz_xConnSetup 32
#define sz_xPixmapFormat 8
#define sz_xDepth 8
#define sz_xVisualType 24
#define sz_xWindowRoot 40
#define sz_xScreen 40
#define sz_xHostEntry 4
#define sz_xCharInfo 12
#define sz_xFontProp 8
#define sz_xQueryFontReply 60
#define sz_xChar2b 2
#define sz_xColorItem 12
#define sz_xrgb 8

typedef unsigned long XID;
typedef unsigned long Mask;
typedef unsigned long Atom;
typedef unsigned long VisualID;
typedef unsigned long Time;
typedef XID Window;
typedef XID Drawable;
typedef XID Font;
typedef XID Pixmap;
typedef XID Cursor;
typedef XID Colormap;
typedef XID GContext;
typedef XID KeySym;

// X11 connection structures and macros
typedef struct {
    unsigned char byteOrder;
    unsigned char pad;
    unsigned short majorVersion;
    unsigned short minorVersion;
    unsigned short nbytesAuthProto;
    unsigned short nbytesAuthString;
    unsigned short pad2;
} xConnClientPrefix;

typedef struct {
    unsigned char success;
    unsigned char lengthReason; 
    unsigned short majorVersion; 
    unsigned short minorVersion; 
    unsigned short length; 
} xConnSetupPrefix;

typedef struct {
    CARD32 release; 
    CARD32 ridBase; 
    CARD32 ridMask; 
    CARD32 motionBufferSize; 
    CARD16 nbytesVendor; 
    CARD16 maxRequestSize; 
    CARD8 numRoots; 
    CARD8 numFormats; 
    CARD8 imageByteOrder; 
    CARD8 bitmapBitOrder; 
    CARD8 bitmapScanlineUnit; 
    CARD8 bitmapScanlinePad; 
    CARD8 minKeyCode; 
    CARD8 maxKeyCode; 
    CARD32 pad2; 
} xConnSetup;

typedef struct {
	Window root;
	Colormap cmap;
	unsigned int white_pixel;
	unsigned int black_pixel;
	int max_maps, min_maps;
	int backing_store;
	Bool save_unders;
	long root_input_mask;
} Screen;

typedef struct {
	int depth, nvisuals;
	Visual *visuals;
} Depth;

typedef struct {
	unsigned long event_mask;	
	int (*handler)();
} EventHandler;

typedef struct _XDisplay {
	WiFiClient client; // TCP client for X11 connection
	WiFiUDP udp; // UDP client for X11 events
	char *display_name; // display name as host:port.screen
	int fd; // file descriptor of the TCP client
	int proto_major_version, proto_minor_version; // protocol version numbers
	char *vendor; // vendor string
	XID resource_base, resource_mask, resource_id; // resource ID management
	int min_keycode, max_keycode; // keycode range
	int nscreens; // number of screens
	Screen *screens; // array of screens
	Screen *default_screen; // default screen pointer
	int default_screen_no; // default screen number
	Atom *atoms; // atom cache array
	int atom_count, atom_size; // atom cache size and count
	unsigned long request; // request number counter
	char *last_request_read; // pointer to last request read from server
	char *buffer, *bufptr, *bufmax; // output buffer and pointers
	char *buffer_size; // output buffer size
	KeySym *keysyms; // keysym mapping array
	int keysyms_per_keycode; // keysym mapping size per keycode
	char *server_ip_address;// server IP address in dot notation (e.g. "192.168.1.1")
	int server_port;// server port number (e.g. 6000)
	EventHandler *event_vec[128]; // event vector for each event type (0-127)
	EventHandler *wire_vec[128]; // wire event vector for each event type (0-127)
} Display;

// Output buffer macros

// GetReq(name, req) is used to obtain a pointer to a structure into which the arguments for a request can be stored. The pointer returned points to an area of memory in the display's output buffer. The length field of the request is automatically set to the size (in 4 byte quantities) of the request structure and the request type field is set to the protocol request type.
#define GetReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(x##name##Req)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (x##name##Req *)(dpy->bufptr);\
	req->reqType = X_##name;\
	req->length = (SIZEOF(x##name##Req))>>2;\
	dpy->bufptr += SIZEOF(x##name##Req);\
	dpy->request++

// GetReqExtra(name, n, req) is used for requests that have extra data after a fixed size request structure. The length field of the request is automatically set to the size of the request structure plus n bytes and the request type field is set to the protocol request type.
#define GetReqExtra(name, n, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(x##name##Req) + n) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (x##name##Req *)(dpy->bufptr);\
	req->reqType = X_##name;\
	req->length = (SIZEOF(x##name##Req) + n)>>2;\
	dpy->bufptr += SIZEOF(x##name##Req) + n;\
	dpy->request++

// GetEmptyReq(name, req) is used for requests that have no arguments at all. The length field of the request is automatically set to 1 and the request type field is set to the protocol request type.
#define GetEmptyReq(name, req) \
        WORD64ALIGN\
	if ((dpy->bufptr + SIZEOF(xReq)) > dpy->bufmax)\
		_XFlush(dpy);\
	req = (xReq *) (dpy->bufptr);\
	req->reqType = X_##name;\
	req->length = 1;\
	dpy->bufptr += SIZEOF(xReq);\
	dpy->request++

// Data macros

// Data16(dpy, data, len) is used to copy 16-bit data to the output buffer. The data argument must be a pointer to an array of CARD16s and the len argument must be the number of CARD16s to be copied. The data is padded if necessary to a 32-bit boundary.
#define Data16(dpy, data, len) \
	if (dpy->bufptr + ((len) << 1) <= dpy->bufmax) {\
		memcpy(dpy->bufptr, data, len << 1);\
		dpy->bufptr += ((len) << 1);\
		if (len & 1) {\
			*dpy->bufptr = 0;\
			dpy->bufptr += 2;\
		}\
	} else\
		_XSend(dpy, (char *)data, len << 1)

// Data32(dpy, data, len) is used to copy 32-bit data to the output buffer. The data argument must be a pointer to an array of CARD32s and the len argument must be the number of CARD32s to be copied.
#define Data32(dpy, data, len) \
	if (dpy->bufptr + ((len) << 2) <= dpy->bufmax) {\
		memcpy(dpy->bufptr, data, len << 2);\
		dpy->bufptr += (len << 2);\
	} else\
		_XSend(dpy, (char *)data, len << 2)

// OneDataCard32(dpy, value) is used to copy a single 32-bit value to the output buffer. The value argument must be a CARD32. This macro is useful for requests that have a variable number of 32-bit values after a fixed size request structure.
#define OneDataCard32(dpy, value) \
	if (dpy->bufptr + 4 <= dpy->bufmax) {\
		*(CARD32 *)dpy->bufptr = value;\
		dpy->bufptr += 4;\
	} else\
		_XSend(dpy, (char *)&value, 4)

// X11 functions prototypes
Display *XOpenDisplay(const char *display_name);
int XCloseDisplay(Display *display);
int XFlush(Display *display);
int XSync(Display *display, Bool discard);
int XPending(Display *display);
int XNextEvent(Display *display, XEvent *event);
int XPeekEvent(Display *display, XEvent *event);
int XWindowEvent(Display *display, Window w, long mask, XEvent *event);
int XPutBackEvent(Display *display, XEvent *event);
int XEventsQueued(Display *display, int mode);
Bool XCheckWindowEvent(Display *display, Window w, long mask, XEvent *event);
Bool XCheckTypedWindowEvent(Display *display, Window w, int type, XEvent *event);
Bool XCheckMaskEvent(Display *display, long mask, XEvent *event);
Bool XCheckTypedEvent(Display *display, int type, XEvent *event);

// More functions to be added ...

// X11 functions implementations
Display *XOpenDisplay(const char *display_name) {
	// Allocate a display structure
	Display *dpy = (Display *)malloc(sizeof(Display));
	if (dpy == NULL) {
		return NULL;
	}

	// Parse the display name as host:port.screen
	char *host, *port, *screen;
	host = strdup(display_name); // make a copy of the display name
	if (host == NULL) {
		free(dpy);
		return NULL;
	}
	port = strchr(host, ':'); // find the first colon
	if (port == NULL) {
		free(host);
		free(dpy);
		return NULL;
	}
	*port++ = '\0'; // terminate the host and advance the port
	screen = strchr(port, '.'); // find the first dot
	if (screen == NULL) {
		free(host);
		free(dpy);
		return NULL;
	}
	*screen++ = '\0'; // terminate the port and advance the screen

	// Convert the port and screen to integers
	int port_num = atoi(port) + 6000; // add 6000 to get the actual port number
	int screen_num = atoi(screen);

	// Connect to the X11 server using TCP
	if (!dpy->client.connect(host, port_num)) {
		free(host);
		free(dpy);
		return NULL;
	}

	// Get the file descriptor of the TCP client
	dpy->fd = dpy->client.fd();

	// Set the display name and server address and port
	dpy->display_name = strdup(display_name);
	dpy->server_ip_address = strdup(host);
	dpy->server_port = port_num;

	free(host); // free the host copy

	// Send the initial connection request
	xConnClientPrefix client;
	client.byteOrder = 0x42; // MSB first
	client.pad = 0;
	client.majorVersion = X_PROTOCOL;
	client.minorVersion = X_PROTOCOL_REVISION;
	client.nbytesAuthProto = 0; // no authentication for now
	client.nbytesAuthString = 0; // no authentication for now
	client.pad2 = 0;

	dpy->client.write((uint8_t *)&client, sz_xConnClientPrefix);

	// Wait for the connection reply
	xConnSetupPrefix prefix;
	if (dpy->client.readBytes((char *)&prefix, sz_xConnSetupPrefix) != sz_xConnSetupPrefix) {
		XCloseDisplay(dpy);
		return NULL;
	}

	if (prefix.success == 0) {
		XCloseDisplay(dpy);
		return NULL;
	}

	int setuplength = prefix.length << 2; // length is in 4 byte units

	char *setupdata = (char *)malloc(setuplength); // allocate memory for setup data
	if (setupdata == NULL) {
		XCloseDisplay(dpy);
		return NULL;
	}

	if (dpy->client.readBytes(setupdata, setuplength) != setuplength) { // read setup data
		free(setupdata); // free setup data
		XCloseDisplay(dpy);
		return NULL;
	}

	// Parse the setup data
	xConnSetup *setup = (xConnSetup *)setupdata;
	dpy->proto_major_version = prefix.majorVersion;
	dpy->proto_minor_version = prefix.minorVersion;
	dpy->release = setup->release;
	dpy->resource_base = setup->ridBase;
	dpy->resource_mask = setup->ridMask;
	dpy->min_keycode = setup->minKeyCode;
	dpy->max_keycode = setup->maxKeyCode;

	char *p = setupdata + sz_xConnSetup; // pointer to the rest of setup data

	// Skip the vendor string
	dpy->vendor = (char *)malloc(setup->nbytesVendor + 1); // allocate memory for vendor string
	if (dpy->vendor == NULL) {
		free(setupdata); // free setup data
		XCloseDisplay(dpy);
		return NULL;
	}
	memcpy(dpy->vendor, p, setup->nbytesVendor); // copy vendor string
	dpy->vendor[setup->nbytesVendor] = '\0'; // terminate vendor string
	p += ((setup->nbytesVendor + 3) & ~3); // advance pointer and pad to 4 byte boundary

	// Skip the pixmap formats
	p += (setup->numFormats * sz_xPixmapFormat);

	// Parse the screens
	dpy->nscreens = setup->numRoots; // number of screens
	dpy->screens = (Screen *)malloc(dpy->nscreens * sizeof(Screen)); // allocate memory for screens array
	if (dpy->screens == NULL) {
		free(setupdata); // free setup data
		XCloseDisplay(dpy);
		return NULL;
	}

	for (int i = 0; i < dpy->nscreens; i++) { // for each screen
		xWindowRoot *root = (xWindowRoot *)p; // get the root window structure
		Screen *scr = &dpy->screens[i]; // get the screen pointer

		scr->root = root->windowId; // root window ID
		scr->cmap = root->defaultColormap; // default colormap ID
		scr->white_pixel = root->whitePixel; // white pixel value
		scr->black_pixel = root->blackPixel; // black pixel value
		scr->max_maps = root->maxInstalledMaps; // maximum colormap entries
		scr->min_maps = root->minInstalledMaps; // minimum colormap entries
		scr->backing_store = root->backingStore; // backing store support
		scr->save_unders = root->saveUnders; // save unders support
		scr->root_input_mask = root->currentInputMask; // root input mask

		p += sz_xWindowRoot; // advance pointer

		int ndepths = root->nDepths; // number of depths

		for (int j = 0; j < ndepths; j++) { // for each depth
			xDepth *depth = (xDepth *)p; // get the depth structure

			if (depth->depth == root->rootDepth) { // if this is the root depth
				scr->depth = depth->depth; // set the screen depth
				scr->nvisuals = depth->nVisuals; // set the number of visuals
				scr->visuals = (Visual *)malloc(scr->nvisuals * sizeof(Visual)); // allocate memory for visuals array
				if (scr->visuals == NULL) {
					free(setupdata); // free setup data
					XCloseDisplay(dpy);
					return NULL;
				}

				xVisualType *visual = (xVisualType *)(p + sz_xDepth); // get the first visual structure

				for (int k = 0; k < scr->nvisuals; k++) { // for each visual
					scr->visuals[k].visualid = visual->visualID; // set the visual ID
					scr->visuals[k].class = visual->class; // set the visual class
					scr->visuals[k].bits_per_rgb = visual->bitsPerRGB; // set the bits per RGB value
					scr->visuals[k].map_entries = visual->colormapEntries; // set the colormap entries
					scr->visuals[k].red_mask = visual->redMask; // set the red mask
					scr->visuals[k].green_mask = visual->greenMask; // set the green mask
					scr->visuals[k].blue_mask = visual->blueMask; // set the blue mask

					visual++; // advance to the next visual structure
				}
			}

			p += sz_xDepth + (depth->nVisuals * sz_xVisualType); // advance pointer
		}
	}

	// Set the default screen pointer and number
	dpy->default_screen = &dpy->screens[screen_num];
	dpy->default_screen_no = screen_num;

	// Initialize other fields of the display structure
	dpy->resource_id = dpy->resource_base;
	dpy->request = 0;
	dpy->last_request_read = NULL;
	dpy->buffer = NULL;
	dpy->bufptr = NULL;
	dpy->bufmax = NULL;
	dpy->buffer_size = 0;
	dpy->atoms = NULL;
	dpy->atom_count = 0;
	dpy->atom_size = 0;
	dpy->keysyms = NULL;
	dpy->keysyms_per_keycode = 0;

	for (int i = 0; i < 128; i++) { // for each event type
		dpy->event_vec[i] = NULL; // set the event vector to NULL
		dpy->wire_vec[i] = NULL; // set the wire event vector to NULL
	}

	free(setupdata); // free setup data

	return dpy; // return the display pointer
}

int XCloseDisplay(Display *display) {
	// Check if the display pointer is valid
	if (display == NULL) {
		return 0;
	}

	// Send a close display request to the server
	xReq req;
	req.reqType = X_CloseDisplay;
	req.length = 1;

	display->client.write((uint8_t *)&req, sz_xReq);

	// Close the TCP and UDP clients
	display->client.stop();
	display->udp.stop();

	// Free the memory allocated for the display structure
	free(display->display_name);
	free(display->vendor);
	free(display->server_ip_address);
	free(display->screens);
	free(display->atoms);
	free(display->keysyms);
	free(display->buffer);

	free(display);

	return 1;
}

int XFlush(Display *display) {
	// Check if the display pointer is valid
	if (display == NULL) {
		return 0;
	}

	// Check if there is any data in the output buffer
	if (display->bufptr > display->buffer) {
		// Send the data to the server
		int len = display->bufptr - display->buffer; // length of data in bytes
		display->client.write((uint8_t *)display->buffer, len);

		// Reset the buffer pointers
		display->bufptr = display->buffer;
		display->last_request_read = display->buffer;
	}

	return 1;
}

int XSync(Display *display, Bool discard) {
	// Check if the display pointer is valid
	if (display == NULL) {
		return 0;
	}

	XFlush(display); // flush the output buffer

	if (discard) {
		// Discard all events in the queue
		while (XPending(display) > 0) {
			XEvent event;
			XNextEvent(display, &event); // read and ignore the next event
		}
	} else {
		// Wait for the server to process all requests
		xGetInputFocusReply rep; // dummy reply
		XGetInputFocus(display, &rep); // send a dummy request and wait for reply
	}

	return 1;
}

int XPending(Display *display) {
	// Check if the display pointer is valid
	if (display == NULL) {
		return 0;
	}

	XFlush(display); // flush the output buffer

	// Check if there are any UDP packets available
	int n = display->udp.parsePacket();

	return n; // return the number of bytes available
}

int XNextEvent(Display *display, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return 0;
	}

	XFlush(display); // flush the output buffer

	// Wait for a UDP packet from the server
	display->udp.read((uint8_t *)event, sizeof(XEvent)); // read the event data

	// Call the appropriate event handler if any
	if (display->event_vec[event->type]) {
		display->event_vec[event->type]->handler(display, event);
	}

	return 1;
}

int XPeekEvent(Display *display, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return 0;
	}

	XFlush(display); // flush the output buffer

	// Wait for a UDP packet from the server
	display->udp.read((uint8_t *)event, sizeof(XEvent)); // read the event data

	// Put back the event to the queue
	XPutBackEvent(display, event);

	return 1;
}

int XWindowEvent(Display *display, Window w, long mask, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return 0;
	}

	do {
		XNextEvent(display, event); // get the next event
	} while (event->xany.window != w || !(event->xany.type & mask)); // loop until the event matches the window and the mask

	return 1;
}

int XPutBackEvent(Display *display, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return 0;
	}

	// Send back the event data to the server as a UDP packet
	display->udp.beginPacket(display->server_ip_address, display->server_port);
	display->udp.write((uint8_t *)event, sizeof(XEvent));
	display->udp.endPacket();

	return 1;
}

int XEventsQueued(Display *display, int mode) {
	// Check if the display pointer is valid
	if (display == NULL) {
		return 0;
	}

	XFlush(display); // flush the output buffer

	if (mode == QueuedAlready) {
		// Return the number of events already in the queue
		return display->udp.available();
	} else {
		// Wait for the server to process all requests and return the number of events in the queue
		XSync(display, mode == QueuedAfterFlush);
		return display->udp.available();
	}
}

Bool XCheckWindowEvent(Display *display, Window w, long mask, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return False;
	}

	XFlush(display); // flush the output buffer

	if (XPending(display) > 0) { // if there is an event in the queue
		XPeekEvent(display, event); // peek the event
		if (event->xany.window == w && (event->xany.type & mask)) { // if the event matches the window and the mask
			XNextEvent(display, event); // remove the event from the queue
			return True; // return true
		}
	}

	return False; // return false
}

Bool XCheckTypedWindowEvent(Display *display, Window w, int type, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return False;
	}

	XFlush(display); // flush the output buffer

	if (XPending(display) > 0) { // if there is an event in the queue
		XPeekEvent(display, event); // peek the event
		if (event->xany.window == w && event->xany.type == type) { // if the event matches the window and the type
			XNextEvent(display, event); // remove the event from the queue
			return True; // return true
		}
	}

	return False; // return false
}

Bool XCheckMaskEvent(Display *display, long mask, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return False;
	}

	XFlush(display); // flush the output buffer

	if (XPending(display) > 0) { // if there is an event in the queue
		XPeekEvent(display, event); // peek the event
		if (event->xany.type & mask) { // if the event matches the mask
			XNextEvent(display, event); // remove the event from the queue
			return True; // return true
		}
	}

	return False; // return false
}

Bool XCheckTypedEvent(Display *display, int type, XEvent *event) {
	// Check if the display pointer and the event pointer are valid
	if (display == NULL || event == NULL) {
		return False;
	}

	XFlush(display); // flush the output buffer

	if (XPending(display) > 0) { // if there is an event in the queue
		XPeekEvent(display, event); // peek the event
		if (event->xany.type == type) { // if the event matches the type
			XNextEvent(display, event); // remove the event from the queue
			return True; // return true
		}
	}

	return False; // return false
}

// Helper functions

// _XFlush is a private function that flushes the output buffer and reallocates it if necessary
static int _XFlush(Display *display) {
	// Check if the display pointer is valid
	if (display == NULL) {
		return 0;
	}

	// Check if there is any data in the output buffer
	if (display->bufptr > display->buffer) {
		// Send the data to the server
		int len = display->bufptr - display->buffer; // length of data in bytes
		display->client.write((uint8_t *)display->buffer, len);

		// Reset the buffer pointers
		display->bufptr = display->buffer;
		display->last_request_read = display->buffer;
	}

	// Check if the buffer size is too small
	if (display->buffer_size < BUFSIZE) {
		// Reallocate the buffer with a larger size
		display->buffer_size = BUFSIZE;
		display->buffer = (char *)realloc(display->buffer, display->buffer_size);
		if (display->buffer == NULL) {
			return 0;
		}
		display->bufptr = display->buffer;
		display->bufmax = display->buffer + display->buffer_size;
	}

	return 1;
}

// _XSend is a private function that sends data to the server and flushes the output buffer if necessary
static int _XSend(Display *display, char *data, long len) {
	// Check if the display pointer and the data pointer are valid
	if (display == NULL || data == NULL) {
		return 0;
	}

	// Check if the data fits in the output buffer
	if (len > display->bufmax - display->bufptr) {
		// Flush the output buffer
		if (!_XFlush(display)) {
			return 0;
		}
	}

	// Check if the data is larger than the output buffer
	if (len > display->buffer_size) {
		// Send the data directly to the server
		display->client.write((uint8_t *)data, len);
	} else {
		// Copy the data to the output buffer
		memcpy(display->bufptr, data, len);
		display->bufptr += len;
	}

	return 1;
}

// _XAllocID is a private function that allocates a new resource ID
static XID _XAllocID(Display *display) {
	// Check if the display pointer is valid
	if (display == NULL) {
		return 0;
	}

	display->request++; // increment the request number
	return (display->request | display->resource_base); // return the resource ID
}