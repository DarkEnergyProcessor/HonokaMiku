How to compile
==============

This project can be compiled under Windows 7 SDK/Visual Studio 2010 (or later), and GCC (with G++) 4.8.1 (or later)

## Linux/Mac OS X

In Linux, install GCC C++ compiler then type `make`

The Makefile script is only tested in Ubuntu & Windows (MinGW).

Cross-compilation is supported by adding `PREFIX=<compiler name prefix>`. Example: `make PREFIX=i686-linux-gnu`

## Linux (Android)

Set environment variable `NDK_BUILD` to `ndk-build` script path, or append NDK directory to your `PATH` environment variable.

Then, type `make ndk`

Windows users: **Cygwin** might be required, but I tested it with MinGW (Msys) tools and it seems to work.

## Windows

### Visual Studio 2010 or above
Open `sln/HonokaMiku.sln` and compile. If you have Windows SDK v7.1 x64 installed, you can use it to compile 64-bit version of HonokaMiku by selecting the Solution Platforms from Win32 to x64. Additionally, you can compile 64-bit version with VS2012 or later by changing the Platform Toolset for corresponding platform to corresponding Visual Studio version.

### Visual Studio 2010 or above with MinGW make
Open Visual Studio 2010 command prompt, navigate to this folder, and type `make vscmd`.

### With MinGW/Cygwin
Just follow Linux compiling instructions above.
