#include "clipboard.h"
#include <node_api.h>

#define nameof(function) ((void)function, #function)

napi_value Init(napi_env env, napi_value exports) {
	napi_value fn;

	napi_create_function(env, NULL, 0, PlatformRead, NULL, &fn);
	napi_set_named_property(env, exports, "read", fn);

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
