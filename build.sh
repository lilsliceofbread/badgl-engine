#! /bin/bash

cmake --build build

cd build/example

rm -rf shaders
rm -rf res

cp -r ../../shaders .
cp -r ../../example/res .