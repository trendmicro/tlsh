#!/bin/sh

# This make file packages and publishes the Python package for TLSH.
# For building the C++ code, run the make file in the project root directory.

OPTION=$1

if [ "$OPTION" = "package" ]; then
  echo 'Building tlsh Python package'
  cp -r ../src ./
  cp -r ../include ./
  cp ../CMakeLists.txt ./
  cp ../LICENSE ./

  python setup.py sdist

  rm -r ./src
  rm -r ./include
  rm ./CMakeLists.txt
  rm ./LICENSE
fi

if [ "$OPTION" = "publish" ]; then
  python setup.py publish
fi