# hal
An interface implemented by Hardware Abstraction Layers.

## Requirements
### Global config
- `HAL_RUN`: a command to run an executable and print kernel log to the standard output
- `HAL_GENIMAGE`: a script to generate a disk image file
- `HAL_GENSTART`: a script to generate applications startup code
- `HAL_START_LANG`: a language of the applications startup code
- `HAL_LINK`: a linker command to link an executable file

### C++ interfaces
C++ API which HALs MUST implement is described in `hal.h`, a header file included in `cpp` package.
