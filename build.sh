#! /bin/bash

cd build
rm -rf shaders
rm -rf res
make
cp -r ../shaders .
cp -r ../res .