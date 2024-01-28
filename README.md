# badgl engine
making a terrible graphics engine/framework to learn c and openGL.

WIP (no documentation, have fun!)

the example program should help to give some idea of the structure of the engine.

press *Esc* to toggle keyboard and mouse input, *P* to toggle wireframe view, and *Q* to quit.

# requirements

- GPU supporting OpenGL 4.3

- CMake

- all compilers should *theoretically* work (gcc tested on linux and windows) 

# compiling

No matter how you choose to compile the library, the `shaders/` folder should be in the same directory as the executable.

*Linux*: Required packages: `libxrandr-dev libglx-dev`. Run `cmake -S . -B build` to configure cmake, then `./build.sh`.

*Windows*: Build with `./build.bat`. If using MinGW GCC compiler is available it will be used. If you want to use a specific compiler you need to edit the cmake command within build.bat, along with the Makefile type.

# using the engine

- If you are using your own resources (e.g. models, textures etc.), specify your paths relative to the executable.