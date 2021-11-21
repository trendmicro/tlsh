#!/bin/sh

####################################################
# process command line options
####################################################

OPTION=$1

CHECKSUM="-DTLSH_CHECKSUM_1B=1"
notest=0
VERBOSEOPTION=""

if test "$OPTION" = "-zerochecksum"
then
	OPTION=$2
	CHECKSUM="-DTLSH_CHECKSUM_NO_EVALUATION=1"
fi
if test "$OPTION" = "-nochecksum"
then
	OPTION=$2
	CHECKSUM="-DTLSH_CHECKSUM_0B=1"
fi
if test "$OPTION" = "-verbose"
then
	OPTION=$2
	VERBOSEOPTION="VERBOSE=1"
fi
if [ "$OPTION" = "-notest" ]; then
	notest=1
fi

####################################################

if [ "$OPTION" = "debug" ]; then
	mkdir -p build/debug
	cd build/debug
	cmake $CHECKSUM -DCMAKE_BUILD_TYPE=Debug ../..
elif [ "$OPTION" = "-shared" ]; then
	mkdir -p build/release
	cd build/release
	cmake $CHECKSUM -DTLSH_SHARED_LIBRARY=1 ../..
else
	mkdir -p build/release
	cd build/release
	cmake $CHECKSUM ../.. 
fi

makecversion=0
if [ "$OPTION" = "-c" ]; then
	makecversion=1
fi

make $VERBOSEOPTION

cd ../../bin
if test ! -f tlsh
then
	ln -s tlsh_unittest tlsh
fi
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
