# libfp

A C++ interface for instances of [Blue Maxima's Flashpoint](https://bluemaxima.org/flashpoint/)

## WIP
Currently this library exists explicitly for use with [CLIFp](https://github.com/oblivioncth/CLIFp) and [FIL](https://github.com/oblivioncth/FIL), and as such its feature set has been implemented sporadically as needed and is largely incomplete. Therefore, there is no documentation available for the interface at this time given it is not ready for general purpose use. That being said, given its bareness and somewhat simplistic nature, you can get a sense of how to use its API by reviewing the code directly if you so desire.

## Source

### Summary

 - C++20
 - CMake 3.21.1
 - Targets Windows 10 and above

### Dependencies
- Qt6
- [Qx](https://github.com/oblivioncth/Qx/)

### Builds
Tested with MSVC2022.

### Details
The source for this project is managed by a sensible CMake configuration that allows for straightforward compilation and consumption of its target(s), either as a sub-project or as an imported package. All required dependencies except for Qt6 are automatically acquired via CMake's FetchContent mechanism.