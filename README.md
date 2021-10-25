Read / write from the native OS clipboard in Node.js

## Features
- Simple API usage and Error Handling
- Fast / Direct OS Calls using C
- Full Unicode Support (Non-English characters, Emojis, etc.)

## Planned Features
- Linux, Mac OS, FreeBSD Support
- Optional TypeScript Definitions

## Example Usage
 ```js
const clipboard = require("native-clip");

// Returns a string of clipboard contents
clipboard.read();

// Copies "Test 1234" to the clipboard
clipboard.write("Test 1234");

// Full Unicode Support
clipboard.write("🥑");
 ```
