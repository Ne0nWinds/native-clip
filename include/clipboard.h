#include <node_api.h>

void PlatformInit();
napi_value PlatformRead(napi_env env, napi_callback_info info);
napi_value PlatformWrite(napi_env env, napi_callback_info info);

