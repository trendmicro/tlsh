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
makecversion=0
if [ $# -eq 1 -a "$1" = "-c" ]; then
  makecversion=1
fi

make
cd ../../bin
cmake -E create_symlink tlsh_unittest tlsh
cd -

echo
echo "==========="
echo "Cmake Tests"
echo "==========="
make test

cd ../..

if test $makecversion = 1
then
  echo
  echo "==========="
  echo "make c standalone version"
  echo "==========="
  cd src_c_standalone
  make
  cd ..
fi

echo
echo "====================="
echo "tests on example data"
echo "====================="
cd Testing
./test.sh | grep -E "(^test|^passed|error|^Running|Scenario)"
cd ..
