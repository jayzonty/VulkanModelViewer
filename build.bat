if not exist build\ (
    mkdir build\
)

cd build\
cmake -G "MinGW Makefiles" ..

cmake --build .

copy compile_commands.json ..\compile_commands.json