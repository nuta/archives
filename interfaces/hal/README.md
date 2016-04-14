# hal
An interface implemented by Hardware Abstraction Layers.

## Requirements
### Global config
- `HAL_LINK`: a linker command to link an executable file
- `HAL_TEST`: a command to run an executable and print kernel log to the standard output
- `HAL_DEBUG`: same as `HAL_TEST` except launching a debugger
- `HAL_GENSTART`: a script to generate the applications startup

### C interfaces
C API which HALs MUST implement is described in `hal.h`, a header file included in `cpp` package.
