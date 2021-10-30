#include "clipboard.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

typedef enum {
	ATOM_UTF8 = 0,
	ATOM_XASTRING,
	ATOM_CLIPBOARD,
    ATOM_NATIVE_CLIP,
	ATOM_ARRAY_LENGTH
} AtomConstantIndex;

static Atom AtomConstants[ATOM_ARRAY_LENGTH] = {};

static Display *X11Display;
static Window X11WindowHandle;
static char *CurrentClipboardString = NULL;
static size_t ClipboardStringLength = 0;
static bool isUTF8 = false;

void PlatformInit() {
	X11Display = XOpenDisplay(NULL);
	if (!X11Display) return;
	AtomConstants[ATOM_UTF8] = XInternAtom(X11Display, "UTF8_STRING", 0);
	AtomConstants[ATOM_XASTRING] = XA_STRING;
	AtomConstants[ATOM_CLIPBOARD] = XInternAtom(X11Display, "CLIPBOARD", 0);
    AtomConstants[ATOM_NATIVE_CLIP] = XInternAtom(X11Display, "NATIVE_CLIP", 0);

    Window root = RootWindow(X11Display, DefaultScreen(X11Display));
    X11WindowHandle = XCreateSimpleWindow(X11Display, root, -10, -10, 1, 1, 0, 0, 0);
}

static int IsClipboardEvent(Display *Dpy, XEvent *Event, XPointer Pointer) {
    bool result = Event->type &&
        Event->xproperty.state == PropertyNewValue &&
        Event->xproperty.window == ((XEvent *)Pointer)->xselection.requestor &&
        Event->xproperty.atom == ((XEvent *)Pointer)->xselection.property;
    return result;
}

void SetClipboardString(AtomConstantIndex EncodingIndex) {
    Atom Encoding = AtomConstants[EncodingIndex];
    XConvertSelection(X11Display, AtomConstants[ATOM_CLIPBOARD], Encoding, AtomConstants[ATOM_NATIVE_CLIP], X11WindowHandle, CurrentTime);

    XEvent Event = {0};
    while (!XCheckTypedWindowEvent(X11Display, X11WindowHandle, SelectionNotify, &Event)) usleep(0);

    if (Event.xselection.property == None) return;

    XEvent NullEvent;
    XCheckIfEvent(X11Display, &NullEvent, IsClipboardEvent, (void *)&Event);

    unsigned long Count = 0;
    int Format = 0;
    Atom Type = {0};
    unsigned long ExtraBytes = 0;
    char *data = 0;
    XGetWindowProperty(X11Display,
            Event.xselection.requestor,
            Event.xselection.property,
            0, LONG_MAX, True, AnyPropertyType,
            &Type, &Format, &Count, &ExtraBytes, (unsigned char **)&data);

    if (Type == Encoding)
    {
        isUTF8 = Encoding == AtomConstants[ATOM_UTF8];
        ClipboardStringLength = strlen(data);
        CurrentClipboardString = calloc(ClipboardStringLength + 1, sizeof(char));
        memcpy(CurrentClipboardString, data, ClipboardStringLength);
    }

    XFree(data);

}

napi_value PlatformRead(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;
	napi_get_null(env, &ReturnValue);

    if (!X11Display) return ReturnValue;

	Window ClipboardOwner = XGetSelectionOwner(X11Display, AtomConstants[ATOM_CLIPBOARD]);
    if (ClipboardOwner == None || X11WindowHandle == ClipboardOwner) {
        if (isUTF8)
            napi_create_string_utf8(env, CurrentClipboardString, ClipboardStringLength, &ReturnValue);
        else
            napi_create_string_latin1(env, CurrentClipboardString, ClipboardStringLength, &ReturnValue);
        return ReturnValue;
    }

    if (CurrentClipboardString) {
        free(CurrentClipboardString);
        CurrentClipboardString = NULL;
        ClipboardStringLength = 0;
    }

    SetClipboardString(ATOM_UTF8);

    if (!CurrentClipboardString)
        SetClipboardString(ATOM_XASTRING);

    if (CurrentClipboardString) {
        if (isUTF8)
            napi_create_string_utf8(env, CurrentClipboardString, ClipboardStringLength, &ReturnValue);
        else
            napi_create_string_latin1(env, CurrentClipboardString, ClipboardStringLength, &ReturnValue);
    }

	return ReturnValue;
}

napi_value PlatformWrite(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;
	napi_get_boolean(env, false, &ReturnValue);

	return ReturnValue;
}
