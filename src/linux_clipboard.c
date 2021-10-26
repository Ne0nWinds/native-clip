#include "clipboard.h"
#include <X11/Xlib.h>
#include <stdio.h>

static unsigned char *GetUTF8Property(Display *display, Window w, Atom p) {
	Atom type;
	int di;
	unsigned long size, dul;
	unsigned char *prop_ret = 0;
	XGetWindowProperty(display, w, p, 0, 0, False, AnyPropertyType, &type, &di, &dul, &size, &prop_ret);
	XFree(prop_ret);
	Atom INCR = XInternAtom(display, "INCR", False);
	if (type == INCR) return 0;
	Atom da;
	XGetWindowProperty(display, w, p, 0, size, False, AnyPropertyType, &da, &di, &dul, &dul, &prop_ret);
    printf("%s", prop_ret);
	return prop_ret;
}

napi_value PlatformRead(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;
	napi_get_null(env, &ReturnValue);

	Display *dpy = XOpenDisplay(NULL);
	if (!dpy) return ReturnValue;

	int screen = DefaultScreen(dpy);
	Window root = RootWindow(dpy, screen);

	Atom sel, utf8;
	sel = XInternAtom(dpy, "CLIPBOARD", False);
	utf8 = XInternAtom(dpy, "UTF8_STRING", False);

	Window target_window = XCreateSimpleWindow(dpy, root, -10, -10, 1, 1, 0, 0, 0);

	Atom target_property = XInternAtom(dpy, "AUTO", False);
	XConvertSelection(dpy, sel, utf8, target_property, target_window, CurrentTime);


	XEvent ev;
	XSelectionEvent *sev;
	for (;;)
	{
		XNextEvent(dpy, &ev);
		switch (ev.type)
		{
			case SelectionNotify: {
				sev = (XSelectionEvent *)&ev.xselection;
				if (sev->property != None) {
					GetUTF8Property(dpy, target_window, target_property);
				}
				return ReturnValue;
			} break;
		}
	}

	return ReturnValue;
}

napi_value PlatformWrite(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;
	napi_get_boolean(env, false, &ReturnValue);

	return ReturnValue;
}
