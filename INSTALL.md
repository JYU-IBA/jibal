# BUILD AND INSTALL INSTRUCTIONS FOR JIBAL (incomplete)

## Minimum requirements:
- Microsoft Windows 8.1 or newer, tested on Windows 10 OR
- Some sane Linux distribution (Arch-based, Debian-based, anything really) OR
- BSD, maybe? OR
- MacOS X, something relatively recent probably AND
- Almost any C compiler (GCC, Clang, MSVC >= 2015) with C99 support
- GNU Scientific Library >= 2.4
- CMake >= 3.13, older versions might work on some systems too

## Installation instructions for Linux / MacOS (also see MacOS specific instructions below):
1. Install *git* and GNU Scientific Library (gsl) using your distributions package manager
    - On Ubuntu: apt install git gsl TODO: check!
    - On Arch: pacman -S git gsl
2. Run the following:

        $ git clone git@gitlab.jyu.fi:iba/homebrew.git # (TODO: this is not currently public)
        $ mkdir build && cd build
        $ cmake ../
        $ make
        $ sudo make install

## Installation instructions for MacOS:
1. Install [Homebrew](https://brew.sh/)
2. Run the following:
    
        $ brew tap iba/homebrew git@gitlab.jyu.fi:iba/homebrew.git # (TODO: this is not currently public)
        $ brew install jibal

4. If you want to DEVELOP Jibal and not just use it, follow Linux instructions above, install gsl using Homebrew or MacPorts

## Installation instructions for Microsoft Windows 10:

1. Install Build tools for [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
    - note that full MSVC is not necessary!
    - also earlier versions (MSVC 2017 and MSVC 2015) can be installed using the same installer
2. Install [CMake](https://cmake.org/download/)
    - Latest stable is always preferred (3.17.0 at the time of writing this)
    - Allow the installer to add CMake to PATH for convenience
3. Install [Git](https://git-scm.com/download/win)
    - Other tools, e.g. GitHub Desktop can be used too
    - Allow the installer to add git to PATH for convenience
4. Install [vcpkg](https://github.com/microsoft/vcpkg)
    - Clone using git, bootstrap as instructed, place e.g. in *C:\vcpkg*
    - Set environment variable *VCPKG_DEFAULT_TRIPLET=x64-windows*  and *VCPKG_ROOT=C:\vcpkg* using Windows Control Panel to build 64-bit packages by default
    - Install these libraries: *gsl*, *getopt*, specifying x64-windows is not necessary if you did the step above
    
            vcpkg.exe install gsl:x64-windows getopt:x64-windows
    
    - Alternatively use *x86-windows* to compile 32-bit libraries. This is not recommended.
5. Clone Jibal repository
6. Build
    - Set up your MSVC environment by running the vcvars64.bat (or opening the *x64 Native Tools Command Prompt for VS2019*)
    - Run the following (from wherever jibal is cloned to):
    
          mkdir build
          cd build
          cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ../
          
    - We use the *-G* option to select the MSVC compiler version. It's not strictly necessary. Use *Visual Studio 15 2017* for  MSVC 2017. Older versions are not supported.
    - It may not be necessary to speficy $CMAKE_TOOLCHAIN_FILE$ explicitly either 
    - Run the following to build the library
    
          msbuild BUILD_ALL.vcxproj
          
    - You can make an installer, it requires [WiX](https://wixtoolset.org/), run the following:
         
          msbuild PACKAGE.vcxproj
        
    - You can install it like any other msi installer, for quick deployment run this:
        
          msiexec /i jibal-0.2.0-win64.msi /qb
    
        
        