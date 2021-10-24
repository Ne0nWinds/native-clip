#include "clipboard.h"
#include <node_api.h>

#define nameof(function) ((void)function, #function)

napi_value read(napi_env env, napi_callback_info info) {
	char *ClipboardData = PlatformGetClipboard();

	napi_value ReturnValue;
	napi_create_string_utf8(env, ClipboardData, NAPI_AUTO_LENGTH, &ReturnValue);

	if (ClipboardData) PlatformCloseClipboard();
	return ReturnValue;
}

napi_value Init(napi_env env, napi_value exports) {
	napi_value fn;

	napi_create_function(env, NULL, 0, read, NULL, &fn);
	napi_set_named_property(env, exports, "read", fn);

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
