 This is an npm package to read / write from the native OS clipboard

 Example Usage:
 ```js
const clipboard = require("native-clip");

// Returns a string of clipboard contents
clipboard.read();

// Copies "Test 1234" to the clipboard
clipboard.write("Test 1234");

// Full Unicode Support
clipboard.write("🥑");
 ```
