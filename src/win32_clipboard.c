#include "clipboard.h"
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

napi_value PlatformRead(napi_env env, napi_callback_info info) {
	wchar_t *ClipboardData = 0;
	napi_value ReturnValue;
	napi_get_null(env, &ReturnValue);

	if (OpenClipboard(NULL)) {
		ClipboardData = GetClipboardData(CF_UNICODETEXT);
		if (ClipboardData) {
			GlobalLock(ClipboardData);
			napi_create_string_utf16(env, ClipboardData, NAPI_AUTO_LENGTH, &ReturnValue);
			GlobalUnlock(ClipboardData);
		}
		CloseClipboard();
	}

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

	if (OpenClipboard(NULL)) {
		size_t BufferSize = 0;
		napi_get_value_string_utf16(env, argv, NULL, 1, &BufferSize);
		if (!BufferSize) return ReturnValue;
		BufferSize = (BufferSize + 1) * sizeof(char16_t);
		HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, BufferSize);
		size_t BytesRead = 0;
		napi_get_value_string_utf16(env, argv, GlobalLock(hg), BufferSize, &BytesRead);
		GlobalUnlock(hg);

		if (BytesRead) {
			HANDLE ClipboardData = SetClipboardData(CF_UNICODETEXT, hg);
			napi_get_boolean(env, ClipboardData != NULL, &ReturnValue);
		}
		CloseClipboard();
	}

	return ReturnValue;
}
