:: use this if you want to set your compiler manually
::cmake -DCMAKE_C_COMPILER=[compiler] -DCMAKE_CXX_COMPILER=[compiler] -S . -B build

:: detect if gcc exists and if so use it; else use default
g++ --version
if %errorlevel%==0 (cmake -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -G "MSYS Makefiles" -S . -B build) else (cmake -S . -B build)

cmake --build build

cd build\example

rmdir /s/q shaders
rmdir /s/q res
mkdir shaders
mkdir res

xcopy /e/i ..\..\shaders .\shaders
xcopy /e/i ..\..\example\res .\res