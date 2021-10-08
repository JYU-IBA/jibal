REM This script probably doesn't work, but it gives you an idea how the process should work
REM Run this bat script using x64 Native Tools Command Prompt for VS 2019 or run the following bat before running this script
REM "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
REM In case you run into errors, delete the build directory or at least delete the CMakeCache.txt file
del CMakeCache.txt
cmake -G "Visual Studio 16 2019" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake ../
REM The following will make a package
cmake --build . --target PACKAGE --config Release
REM The following (uncommented) will just build
REM cmake --build . --target ALL_BUILD --config Release
