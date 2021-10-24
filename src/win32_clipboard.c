#include "clipboard.h"
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

napi_value PlatformRead(napi_env env, napi_callback_info info) {
	wchar_t *ClipboardData = 0;
	napi_value ReturnValue;

	if (OpenClipboard(NULL)) {
		ClipboardData = GetClipboardData(CF_UNICODETEXT);
		GlobalLock(ClipboardData);
		napi_create_string_utf16(env, ClipboardData, NAPI_AUTO_LENGTH, &ReturnValue);
		GlobalUnlock(ClipboardData);
		CloseClipboard();
	} else {
		napi_get_null(env, &ReturnValue);
	}

	return ReturnValue;
}

static HANDLE Heap;

napi_value PlatformWrite(napi_env env, napi_callback_info info) {
	napi_value ReturnValue;

	size_t argc = 1;
	napi_value argv = 0;
	napi_get_cb_info(env, info, &argc, &argv, NULL, NULL);

	if (argc == 0) {
		napi_throw_error(env, NULL, "Function was called without any arguments");
		goto end;
	}

	if (OpenClipboard(NULL)) {
		EmptyClipboard();
		size_t BufferSize = 0;
		napi_get_value_string_utf16(env, argv, NULL, 1, &BufferSize);
		BufferSize = (BufferSize + 1) * sizeof(char16_t);
		HANDLE Heap = GetProcessHeap();
		void *Buffer = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, BufferSize);
		GlobalLock(Buffer);
		size_t BytesRead = 0;
		napi_get_value_string_utf16(env, argv, Buffer, BufferSize, &BytesRead);
		GlobalUnlock(Buffer);

		if (BytesRead) {
			SetClipboardData(CF_UNICODETEXT, Buffer);
		}

		CloseClipboard();
	}

end:
	napi_get_null(env, &ReturnValue);

	return ReturnValue;
}

void PlatformInit() {
	Heap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
}
