REM This script probably doesn't work, but it gives you an idea how the process should work
"C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\vc\Auxiliary\Build\vcvars64.bat"
mkdir build
cd build
REM In case you run into errors, delete the build directory or at least delete the CMakeCache.txt file
del CMakeCache.txt
cmake -G "Visual Studio 15 2017" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE ../
msbuild ALL_BUILD.vcxproj
