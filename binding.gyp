{
        "targets": [
	{
		"target_name": "NativeClipboardCBinding",
		"include_dirs": [ "./include" ],
		"sources": [ "./src/main.c" ],
		"conditions": [
		["OS=='win'", {
			"sources": [ "./src/win32_clipboard.c" ]
		}],
		["OS=='mac'", {
			"sources": [ "./src/mac_clipboard.m" ],
			"libraries": [ "-framework AppKit" ]
		}],
		["OS=='linux'", {
			"sources": [ "./src/linux_clipboard.c" ],
			"libraries": [ "-lX11" ],
		}],
		["OS=='freebsd'", {
			"sources": [ "./src/freebsd_clipboard.c" ]
		}],
		]
	}
	]
}
