# badgl engine

a ~~bad~~ opengl graphics engine/framework in c

uses glfw & glad, stb_image, cimgui, and assimp

# requirements

- c standard 23

- CMake

- all compilers should *theoretically* work (gcc tested on linux and windows) 

# compiling

*Linux*: Required packages: `libxrandr-dev libglx-dev`. Run `cmake -S . -B build` to configure cmake, then `./build.sh`.

*Windows*: Build with `./build.bat`. If using MinGW GCC compiler is available it will be used. If you want to use a specific compiler you need to edit the cmake command within build.bat, along with the Makefile type.

If you choose to compile another way, make sure the `shaders/` folder is in the same directory as the executable.

# using the engine

- the example program should help to give some idea of the structure of the engine

- press *Esc* to toggle keyboard and mouse input, and *P* to toggle wireframe view

- if you are using your own resources (e.g. models, textures etc.), **specify your paths relative to the executable.**
