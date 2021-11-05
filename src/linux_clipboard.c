#include "clipboard.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

#define len(arr) (sizeof(arr) / sizeof(*arr))

#if defined(__GNUC__) || defined(__clang__)
#define likely(a) __builtin_expect(!!(a), 1)
#define unlikely(a) __builtin_expect(!!(a), 0)
#else
#define likely(a) (a)
#define unlikely(a) (a)
#endif

typedef enum {
	ATOM_UTF8 = 0,
	ATOM_XASTRING,
	ATOM_CLIPBOARD,
	ATOM_NATIVE_CLIP,
	ATOM_CLIPBOARD_MANAGER,
	ATOM_SAVE_TARGETS,
	ATOM_TARGETS,
	ATOM_ARRAY_LENGTH
} AtomConstantIndex;

static Atom AtomConstants[ATOM_ARRAY_LENGTH] = {};

static Display *X11Display;
static Window X11WindowHandle;

static bool X11Init() {
	X11Display = XOpenDisplay(NULL);
	if (unlikely(!X11Display)) return false;
	AtomConstants[ATOM_UTF8] = XInternAtom(X11Display, "UTF8_STRING", 0);
	AtomConstants[ATOM_XASTRING] = XA_STRING;
	AtomConstants[ATOM_CLIPBOARD] = XInternAtom(X11Display, "CLIPBOARD", 0);
	AtomConstants[ATOM_NATIVE_CLIP] = XInternAtom(X11Display, "NATIVE_CLIP", 0);
	AtomConstants[ATOM_CLIPBOARD_MANAGER] = XInternAtom(X11Display, "CLIPBOARD_MANAGER", 0);
	AtomConstants[ATOM_SAVE_TARGETS] = XInternAtom(X11Display, "SAVE_TARGETS", 0);
	AtomConstants[ATOM_TARGETS] = XInternAtom(X11Display, "TARGETS", 0);

	Window root = RootWindow(X11Display, DefaultScreen(X11Display));
	X11WindowHandle = XCreateSimpleWindow(X11Display, root, -10, -10, 1, 1, 0, 0, 0);
	return (X11WindowHandle != None);
}

static void X11Shutdown() {
	if (X11WindowHandle)
		XDestroyWindow(X11Display, X11WindowHandle);
	if (X11Display != NULL)
		XCloseDisplay(X11Display);
	X11WindowHandle = 0;
	X11Display = 0;
}

static int IsClipboardEvent(Display *Dpy, XEvent *Event, XPointer Pointer) {
	bool result = Event->type &&
		Event->xproperty.state == PropertyNewValue &&
		Event->xproperty.window == ((XEvent *)Pointer)->xselection.requestor &&
		Event->xproperty.atom == ((XEvent *)Pointer)->xselection.property;
	return result;
}

static int IsClipboardEventType(Display *Dpy, XEvent *Event, XPointer Pointer) {
	if (Event->xany.window != X11WindowHandle) return False;
	return Event->type == SelectionRequest || Event->type == SelectionNotify || Event->type == SelectionClear;
}

static size_t ClipboardStringLength;
static void SetClipboardString(AtomConstantIndex EncodingIndex, char **ClipboardString) {
	Atom Encoding = AtomConstants[EncodingIndex];
	XConvertSelection(X11Display, AtomConstants[ATOM_CLIPBOARD], Encoding, AtomConstants[ATOM_NATIVE_CLIP], X11WindowHandle, CurrentTime);

	XEvent Event = {0};
	const int MaxWindowEventChecks = 100;
	int i = 0;
	while (!XCheckTypedWindowEvent(X11Display, X11WindowHandle, SelectionNotify, &Event) && i < MaxWindowEventChecks) {
		usleep(0);
		++i;
	}
	if (unlikely(i == MaxWindowEventChecks)) return;

	if (unlikely(Event.xselection.property == None)) return;

	XEvent NullEvent;
	XCheckIfEvent(X11Display, &NullEvent, IsClipboardEvent, (void *)&Event);

	unsigned long Count = 0;
	int Format = 0;
	Atom Type = {0};
	unsigned long ExtraBytes = 0;
	char *data = 0;
	int result = XGetWindowProperty(X11Display, Event.xselection.requestor, Event.xselection.property, 0, LONG_MAX, True, AnyPropertyType, &Type, &Format, &Count, &ExtraBytes, (unsigned char **)&data);

	if (Type == Encoding && result == Success)
	{
		ClipboardStringLength = Count;
		*ClipboardString = data;
	}
}

napi_value PlatformRead(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;
	napi_get_null(env, &ReturnValue);

	if (unlikely(!X11Init()))
		goto end;

	char *ClipboardString = NULL;
	ClipboardStringLength = 0;

	SetClipboardString(ATOM_UTF8, &ClipboardString);
	if (ClipboardString) {
		napi_create_string_utf8(env, ClipboardString, ClipboardStringLength, &ReturnValue);
		XFree(ClipboardString);
		goto end;
	}

	SetClipboardString(ATOM_XASTRING, &ClipboardString);
	if (ClipboardString) {
		napi_create_string_latin1(env, ClipboardString, ClipboardStringLength, &ReturnValue);
		XFree(ClipboardString);
	}

end:
	X11Shutdown();
	return ReturnValue;
}

napi_value PlatformWrite(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;
	napi_get_boolean(env, false, &ReturnValue);

	size_t argc = 1;
	napi_value argv = 0;
	napi_get_cb_info(env, info, &argc, &argv, NULL, NULL);

	if (argc == 0) {
		napi_throw_error(env, NULL, "Function was called without any arguments");
		return ReturnValue;
	}

	bool HasUploaded = false, SavedTargets = false;

	if (unlikely(!X11Init())) goto end;

	XSetSelectionOwner(X11Display, AtomConstants[ATOM_CLIPBOARD], X11WindowHandle, CurrentTime);

	Window ClipboardOwner = XGetSelectionOwner(X11Display, AtomConstants[ATOM_CLIPBOARD]);
	if (unlikely(ClipboardOwner == None || X11WindowHandle != ClipboardOwner)) {
		goto end;
	}

	XConvertSelection(X11Display, AtomConstants[ATOM_CLIPBOARD_MANAGER], AtomConstants[ATOM_SAVE_TARGETS], None, X11WindowHandle, CurrentTime);

	XEvent Event = {0};

	for (int i = 0; i < 1000 && !(HasUploaded && SavedTargets); ++i)
	{

		while (XCheckIfEvent(X11Display, &Event, IsClipboardEventType, NULL))
		{
			switch (Event.type)
			{
				case SelectionRequest: {
					XSelectionRequestEvent *SelectionRequestEvent = (XSelectionRequestEvent *)&Event.xselectionrequest;

					XSelectionEvent EventToSend = {
						.type = SelectionNotify,
						.requestor = SelectionRequestEvent->requestor,
						.selection = SelectionRequestEvent->selection,
						.target = SelectionRequestEvent->target,
						.property = SelectionRequestEvent->property,
						.time = SelectionRequestEvent->time
					};

					char *ClipboardData = NULL;

					if (SelectionRequestEvent->target == AtomConstants[ATOM_TARGETS]) {
						const Atom targets[] = {
							AtomConstants[ATOM_TARGETS],
							AtomConstants[ATOM_SAVE_TARGETS],
							AtomConstants[ATOM_UTF8],
							AtomConstants[ATOM_XASTRING]
						};
						XChangeProperty(X11Display, SelectionRequestEvent->requestor, SelectionRequestEvent->property, XA_ATOM, 32, PropModeReplace, (const unsigned char *)targets, len(targets));
					} else if (SelectionRequestEvent->target == AtomConstants[ATOM_UTF8]) {
						size_t ClipboardLength = 0;
						napi_get_value_string_utf8(env, argv, NULL, 1, &ClipboardLength);
						ClipboardLength += 1;
						ClipboardData = calloc(ClipboardLength, sizeof(char));
						napi_get_value_string_utf8(env, argv, ClipboardData, ClipboardLength, &ClipboardLength);
						XChangeProperty(X11Display, SelectionRequestEvent->requestor, SelectionRequestEvent->property, AtomConstants[ATOM_UTF8], 8, PropModeReplace, (const unsigned char *)ClipboardData, ClipboardLength);
						HasUploaded = true;
					} else if (SelectionRequestEvent->target == AtomConstants[ATOM_XASTRING]) {
						size_t ClipboardLength = 0;
						napi_get_value_string_latin1(env, argv, NULL, 1, &ClipboardLength);
						ClipboardLength += 1;
						ClipboardData = calloc(ClipboardLength, sizeof(char));
						napi_get_value_string_latin1(env, argv, ClipboardData, ClipboardLength, &ClipboardLength);
						XChangeProperty(X11Display, SelectionRequestEvent->requestor, SelectionRequestEvent->property, AtomConstants[ATOM_XASTRING], 8, PropModeReplace, (const unsigned char *)ClipboardData, ClipboardLength);
						HasUploaded = true;
					} else {
						EventToSend.property = None;
					}

					XSendEvent(X11Display, SelectionRequestEvent->requestor, False, 0, (XEvent *)&EventToSend);
					if (ClipboardData) free(ClipboardData);
				} break;

				case SelectionClear: {
					goto end;
				} break;

				case SelectionNotify:
				{
					if (Event.xselection.target == AtomConstants[ATOM_SAVE_TARGETS])
					{
						SavedTargets = true;
					}

					break;
				}
			}
			usleep(0);
		}
	}

end:
	X11Shutdown();
	napi_get_boolean(env, SavedTargets && HasUploaded, &ReturnValue);
	return ReturnValue;
}
