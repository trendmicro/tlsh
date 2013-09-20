#!/bin/sh

mkdir -p build/release
cd build/release
cmake ../..   			# to build debug versions, run "cmake -DCMAKE_BUILD_TYPE=Debug"
make
make test
