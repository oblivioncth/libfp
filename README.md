# libfp

A C++ interface for instances of [Flashpoint Archive](https://flashpointarchive.org/)

[![Dev Builds](https://github.com/oblivioncth/libfp/actions/workflows/push-reaction.yml/badge.svg?branch=dev)](https://github.com/oblivioncth/libfp/actions/workflows/push-reaction.yml)

## WIP
Currently this library exists explicitly for use with [CLIFp](https://github.com/oblivioncth/CLIFp) and [FIL](https://github.com/oblivioncth/FIL), and as such its feature set has been implemented sporadically as needed and is largely incomplete. Therefore, there is no documentation available for the interface at this time given it is not ready for general purpose use. That being said, given its bareness and somewhat simplistic nature, you can get a sense of how to use its API by reviewing the code directly if you so desire.

## Source

### Summary

 - C++20
 - CMake 3.23.0
 - Targets:
    - Windows 10 and above
    - Linux

### Dependencies
- Qt6
- [Qx](https://github.com/oblivioncth/Qx/)
- [OBCMake](https://github.com/oblivioncth/OBCmake)

## Pre-built Releases/Artifacts

Releases and some workflows currently provide builds of libfp in various combinations of platforms and compilers. View the repository [Actions](https://github.com/oblivioncth/libfp/actions) or [Releases](https://github.com/oblivioncth/libfp/releases) to see examples.


### Details
The source for this project is managed by a sensible CMake configuration that allows for straightforward compilation and consumption of its target(s), either as a sub-project or as an imported package. All required dependencies except for Qt6 are automatically acquired via CMake's FetchContent mechanism.