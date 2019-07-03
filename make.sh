#!/bin/sh

####################################################
# process command line options
####################################################

OPTION=$1

notest=0
if [ "$OPTION" = "-notest" ]; then
	notest=1
fi

####################################################

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

cmake --build .

cd ../../bin
if [ -z "${WINDIR}" ]; then
  cmake -E create_symlink tlsh_unittest tlsh
else
  cmake -E copy tlsh_unittest.exe tlsh.exe
fi
cd -

if test $notest = 0
then
	echo
	echo "==========="
	echo "Cmake Tests"
	echo "==========="
	ctest
fi

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

if test $notest = 0
then
	echo
	echo "====================="
	echo "tests on example data"
	echo "====================="
	cd Testing
	./test.sh | grep -E "(^test|^passed|error|^Running|Scenario)"
	cd ..
fi
