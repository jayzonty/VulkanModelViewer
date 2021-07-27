REM Note: setlocal sets environment variables for the local session only
REM Change or comment out these paths depending on the GCC installation
setlocal
CC=C:\msys64\mingw64\bin\gcc.exe
CXX=C:\msys64\mingw64\bin\g++.exe
endlocal

if not exist build\ (
    mkdir build\
)

cd build\
cmake -DCMAKE_BUILD_TYPE="Release" -G "MinGW Makefiles" ..

cmake --build . -j 8

copy compile_commands.json ..\compile_commands.json