# BUILD AND INSTALL INSTRUCTIONS FOR JIBAL

## Minimum requirements:
- Microsoft Windows 8.1 or newer, tested on Windows 10  **OR**
- Some sane Linux distribution (Arch-based, Debian-based, anything really) **OR**
- BSD, maybe? **OR**
- macOS, something relatively recent probably (developer uses Ventura) **AND**
- Almost any C compiler (GCC, Clang, MSVC >= 2015) with C99 support **AND**
- GNU Scientific Library (GSL) >= 2.6  **AND**
- CMake >= 3.15, older versions might work on some systems too
- Git

## Installation instructions for Linux / MacOS (also see MacOS specific instructions below):
1. Install *git*, *CMake*, and *GNU Scientific Library (GSL)* using your distributions package manager
    - On Ubuntu / Debian / Raspberry Pi OS: `apt install git cmake libgsl-dev`
    - On Arch: `pacman -S git gsl cmake`
2. Run the following:

        $ git clone https://github.com/JYU-IBA/jibal.git
        $ mkdir build && cd build
        $ cmake ../
        $ make
        $ sudo make install
        
3. If you get an error when trying to run a program using JIBAL (for example *jibaltool*) that looks like this:
        
        "error while loading shared libraries: libjibal.so.0: cannot open shared object file: No such file or directory"

   You should add the the path where libraries were installed (typically /usr/local/lib) to LD_LIBRARY_PATH for example by adding this line to your .bashrc or .profile
   
        export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"

    Running `ldconfig` as root may also help.

## Installation instructions for MacOS:
1. Install [Homebrew](https://brew.sh/)
2. Run the following:
    
        $ brew tap JYU-IBA/iba
        $ brew install jibal

4. If you want to DEVELOP JIBAL and not just use it, follow Linux instructions above, install gsl using Homebrew or MacPorts

## Installation instructions for Microsoft Windows 10:

1. Install Build tools for [Visual Studio 2019](https://visualstudio.microsoft.com/downloads/)
    - note that full MSVC is not necessary, only the build tools! 
    - MSVC 2022 is not tested, might work
    - also earlier versions (MSVC 2017 and MSVC 2015) can be installed using the same installer
    - other compilers that work with CMake should work too, but are not tested on Windows
2. Install [CMake](https://cmake.org/download/)
    - Latest stable is always preferred (3.25.1 at the time of writing this)
    - Allow the installer to add CMake to PATH for convenience
3. Install [Git](https://git-scm.com/download/win)
    - Other tools, e.g. GitHub Desktop can be used too
    - Allow the installer to add git to PATH for convenience
4. Install [vcpkg](https://github.com/microsoft/vcpkg)
    - Installation of vcpkg is not necessary but it provides *getopt* for Windows and GSL relatively conveniently 
    - Clone using git, bootstrap as instructed, place e.g. in `C:\vcpkg`
    - Set environment variable `VCPKG_DEFAULT_TRIPLET=x64-windows`  and `VCPKG_ROOT=C:\vcpkg` using Windows Control Panel to build 64-bit packages by default
    - More help available on [Microsoft docs](https://docs.microsoft.com/en-us/cpp/build/install-vcpkg?view=msvc-160&tabs=windows)
    - Install these libraries: *gsl*, *getopt*, specifying x64-windows is not necessary if you did the step above
    
            vcpkg.exe install gsl:x64-windows getopt:x64-windows
    
    - Alternatively use *x86-windows* to compile 32-bit libraries. This is not recommended.
    - Use *x64 Native Tools Command Prompt for VS2019* if you get errors related to x86 vs x64 platform issues
5. Clone JIBAL repository (this one)
   6. Build (from command line)
       - Set up your MSVC environment by running the vcvars64.bat (or opening the *x64 Native Tools Command Prompt for VS2019*)
       - Run the following (from wherever JIBAL is cloned to):
    
             mkdir build
             cd build
             cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake ../
          
       - We use the `-G` option to select creation of the MSVC files and to pick the version explicitly. It's not strictly necessary.
       - It may not be necessary to specify `$CMAKE_TOOLCHAIN_FILE$` explicitly either if you followed the instructions
       - Run the following to build the library
    
             cmake --build . --target ALL_BUILD --config Release
          
       - You can make an installer, it requires [WiX](https://wixtoolset.org/), run the following:
         
             cmake --build . --target PACKAGE --config Release
          
       - Alternatively you may use the vcxproj files with Visual Studio or use *msbuild*:
   
             msbuild PACKAGE.vcxproj
          
       - You can install it like any other msi installer, for quick deployment run this:
        
             msiexec /i Jibal-0.X.X-win64.msi /qb
        
        
