#!/bin/sh

if [ $# -eq 1 -a "$1" = "debug" ]; then
  mkdir -p build/debug
  cd build/debug
  cmake -DCMAKE_BUILD_TYPE=Debug ../..
else
  mkdir -p build/release
  cd build/release
  cmake ../.. 
fi

make
make test
