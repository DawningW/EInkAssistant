# ArduinoNative

Library to compile, debug and test platformio projects in native environment without connecting any Arduino board.

- Use this library to perform unit tests in the native environment
- This is not a mocking library, use ArduinoFake for that, just compiles with dummy functions
- Keeps original Arduino code as much as possible
- Stream and Print classes are kept as they are in the original version
