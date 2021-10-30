#include "clipboard.h"

#include <AppKit/AppKit.h>
#include <stdlib.h>

napi_value PlatformRead(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;
	napi_get_null(env, &ReturnValue);

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

	size_t BufferSize = 0;
	napi_get_value_string_utf8(env, argv, NULL, 1, &BufferSize);
	++BufferSize;
	char *Buffer = calloc(BufferSize, sizeof(char));
	size_t BytesRead = 0;
	napi_get_value_string_utf8(env, argv, Buffer, BufferSize, &BytesRead);

	NSPasteboard *Clipboard = [NSPasteboard generalPasteboard];
	[Clipboard clearContents];
	NSString *MacString = [[NSString alloc] initWithBytesNoCopy:(void *)Buffer length: BytesRead encoding:NSUTF8StringEncoding freeWhenDone:FALSE];

	@try {
		BOOL result = [Clipboard setString:MacString forType:NSStringPboardType];
		napi_get_boolean(env, result == YES, &ReturnValue);
	}
	@catch (NSException *exception) { }

	free(Buffer);
	return ReturnValue;
}
