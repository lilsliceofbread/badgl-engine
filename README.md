# badgl-engine
making a terrible graphics "engine" to learn c and opengl.

WIP (no documentation, have fun!)

the example program should help to give some idea of the structure of the engine.

press *Esc* to toggle keyboard and mouse input, *P* to toggle wireframe view, and *Q* to quit.

# requirements

- GPU supporting OpenGL 4.3

- CMake

- all compilers should *theoretically* work (gcc tested on linux and windows) 

# compiling and running the example

*Linux*: Required packages: `libxrandr-dev libglx-dev`. Run `cmake -S . -B build` to configure cmake, then `./build.sh`. Now you can run the example with `./run.sh`

*Windows*: Build with `./build.bat`, then navigate to build/example and run `start game.exe`. NOTE: if the MSVC compiler was used, you may need to run `start .\[Debug or Release]\game.exe` so that the working directory remains in build/example (or just don't use msvc lol).
