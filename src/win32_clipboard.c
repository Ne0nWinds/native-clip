#include "clipboard.h"
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

napi_value PlatformRead(napi_env env, napi_callback_info info) {
	wchar_t *ClipboardData = 0;
	napi_value ReturnValue;

	if (OpenClipboard(NULL)) {
		ClipboardData = GetClipboardData(CF_UNICODETEXT);
		napi_create_string_utf16(env, ClipboardData, NAPI_AUTO_LENGTH, &ReturnValue);
		CloseClipboard();
	} else {
		napi_get_null(env, &ReturnValue);
	}

	return ReturnValue;
}
