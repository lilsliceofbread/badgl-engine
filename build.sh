#! /bin/bash

cd build

make

cd example
rm -rf shaders
rm -rf example/res

cp -r ../../shaders .
cp -r ../../example/res .