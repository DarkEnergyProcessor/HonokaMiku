# How to compile

This project can be compiled under Visual Studio 2010 (or above), and GCC(with G++) 4.8.1 (or above)

### Linux

In Linux, install GCC C++ compiler then type `make`

The Makefile script is only tested in Ubuntu & Windows.

Cross-compilation is supported by adding `PREFIX=<compiler name prefix>`. Example: `make PREFIX=i586-mingw32msvc`

### Linux (Android)

Set environment variable `NDK_BUILD` to `ndk-build` script if it's not in your `PATH` environment variable.

Then, type `make ndk`

Windows users: Cygwin might be required, but I tested it with MinGW (Msys) tools and it seems to work.

### Windows

####Visual Studio 2010 or above
Open `sln/HonokaMiku.sln` and compile. If you have Windows SDK v7.1 x64 installed, you can use it to compile 64-bit version of HonokaMiku.

####Visual Studio 2010 or above with MinGW
Open Visual Studio 2010 command prompt, navigate to this folder, and type `make vscmd`.

####With MinGW
Just follow Linux compiling instructions above.
