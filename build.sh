#! /bin/bash

cd build
rm -rf shaders
make
cp -r ../shaders .