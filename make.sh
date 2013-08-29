#!/bin/sh

mkdir -p build/release
cd build/release
cmake ../..
make
make test
