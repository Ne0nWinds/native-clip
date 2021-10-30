#include "clipboard.h"
#include <X11/Xlib.h>

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

	return ReturnValue;
}
