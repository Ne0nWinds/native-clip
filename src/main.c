#include <clipboard.h>
#include <node_api.h>

napi_value Init(napi_env env, napi_value exports) {
	napi_value fn;

	napi_create_function(env, NULL, 0, PlatformRead, NULL, &fn);
	napi_set_named_property(env, exports, "read", fn);

	napi_create_function(env, NULL, 0, PlatformWrite, NULL, &fn);
	napi_set_named_property(env, exports, "write", fn);

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
