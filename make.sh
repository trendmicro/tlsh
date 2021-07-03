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
elif [ $# -eq 1 -a "$1" = "-shared" ]; then
  mkdir -p build/release
  cd build/release
  cmake -DTLSH_SHARED_LIBRARY=1 ../..
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

if test $notest = 0
then
	echo
	echo "==========="
	echo "Cmake Tests"
	echo "==========="
	make test
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

	echo "====================="
	echo "tests tlsh_pattern"
	echo "====================="
	./test_pattern.sh | grep -E "(^test|^passed|error|^Running|Scenario)"

	echo "====================="
	echo "tests tlsh_parts"
	echo "====================="
	./test_parts.sh | grep -E "(^test|^passed|error|^Running|Scenario)"

	cd ..
fi
