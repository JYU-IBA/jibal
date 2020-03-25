# BUILD AND INSTALL INSTRUCTIONS FOR JIBAL (incomplete)

## Minimum requirements:
- Microsoft Windows 7 or newer, tested on Windows 10 OR
- Some sane Linux distribution (Arch-based, Debian-based, anything really) OR
- BSD, maybe? OR
- MacOS X, something relatively recent probably AND
- Almost any C compiler (GCC, Clang, MSVC >= 2015) with C99 support
- GNU Scientific Library >= 2.4
- CMake >= 3.13, older versions might work on some systems too

## Installation instructions for Linux / MacOS (also see MacOS specific instructions below):
1. Install git and gsl using your distributions package manager
    - On Ubuntu: apt install git gsl
    - On Arch: pacman -S git gsl
2. Run the following:

        $ git clone ????
        $ mkdir build && cd build
        $ cmake ../
        $ make
        $ make install

## Installation instructions for MacOS:
1. Install [Homebrew](https://brew.sh/)
2. brew tap ???????? (TODO: this is not currently public)
3. brew install jibal
4. If you want to DEVELOP Jibal and not just use it, follow Linux instructions above, install gsl using Homebrew or MacPorts

## Installation instructions for Microsoft Windows 10:

1. Install Build tools for [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
    - note that full MSVC is not necessary and also earlier versions (MSVC 2017 and MSVC 2015) can be installed using the same installer
2. Install [CMake](https://cmake.org/download/)
    - Latest stable is always preferred (3.17.0 at the time of writing this)
    - Allow the installer to add CMake to PATH for convenience
3. Install [Git](https://git-scm.com/download/win)
    - Other tools, e.g. GitHub Desktop can be used too
    - Allow the installer to add git to PATH for convenience
4. Install [vcpkg](https://github.com/microsoft/vcpkg)
    - Clone using git, bootstrap as instructed, place e.g. in C:\vcpkg
    - Set environment variable VCPKG_DEFAULT_TRIPLET=x64-windows using Windows Control Panel to build 64-bit packages by default
    - Install these libraries: gsl
    
            vcpkg.exe install gsl
    
    - Make sure 64-bit versions are created (x86-windows means 32-bits)
5. Clone Jibal repository
6. Build
    - Set up your MSVC environment by running the vcvars64.bat (or opening the x64 Native Tools Command Prompt for VS2017)
    - Run the following (from wherever jibal is cloned to):
    
          mkdir build
          cd build
          cmake -G "Visual Studio 15 2017" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE ../
          
    - We use the -G option to select your MSVC compiler version, CMake should detect it
    - We use the -DCMAKE_TOOLCHAIN_FILE as instructed by vcpkg to that the dependencies are included properly
    - We use the CMake feature to export all symbols in the library (default behaviour on Linux/MacOS/BSD)
    - Run the following to build the library
      msbuild BUILD_ALL.vcxproj
    - You can make an installer, it requires [NSIS](https://nsis.sourceforge.io/), run the following:
      msbuild PACKAGE.vcxproj
    - Currently there is an assumption of installation directory (determined by CMake) and installing the library somewhere else means it cannot find the necessary data files. The choice is most likely "C:\Program Files (x86)\jibal", which probably isn't the default of the installer! To do this properly we have to use the Windows registry, but it is not yet implemented.
