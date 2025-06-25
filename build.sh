#!/bin/bash

mkdir -p build

cmake --build build

cd build/example

rm -rf shaders
rm -rf res

cp -r ../../shaders .
cp -r ../../example/res .