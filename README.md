# badgl engine

a ~~bad~~ opengl graphics engine/framework in c

uses glfw & glad, stb_image, cimgui, and assimp

# requirements

- c standard 23

- CMake

- all compilers should *theoretically* work (gcc tested on linux and windows) 

# compiling

a bash/batch script is used to build and then move resources files into the same directory as the executable. if you choose to compile another way, make sure the `shaders/` folder is in the same directory as the executable.

*Linux*: required packages: `sudo apt-get install libxcursor-dev libxi-dev libxinerama-dev libx11-dev libxrandr-dev libglx-dev libgl-dev`. run `cmake -S . -B build` to configure cmake, then `./build.sh`.

*Windows*: build with `./build.bat`. if using MinGW GCC compiler is available it will be used. if you want to use a specific compiler you need to edit the cmake command within build.bat, along with the Makefile type.

*macOS*: unsupported

# using the engine

- the example program should help to give some idea of the structure of the engine

- press *esc* to uncapture keyboard and mouse input

- if you are using your own resources (e.g. models, textures etc.), **specify your paths relative to the executable.**
