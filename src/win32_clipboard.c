#include "clipboard.h"
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

char *PlatformGetClipboard() {
	char *ClipboardData = 0;
	if (OpenClipboard(NULL)) {
		ClipboardData = (char *)GetClipboardData(CF_TEXT);
	}
	return ClipboardData;
}

void PlatformCloseClipboard() {
	CloseClipboard();
}
