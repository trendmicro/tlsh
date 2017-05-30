#!/bin/bash

echoerr() { echo "$@" 1>&2; }

## Set LD_LIBRARY_PATH so that tlsh can pick up the tlsh shared library
export LD_LIBRARY_PATH=../lib:$LD_LIBRARY_PATH
BASEDIR=$(dirname $0)
pushd $BASEDIR > /dev/null

TMP="tmp"
HASH=`../bin/tlsh_version | head -1 | cut -f1`
CHKSUM=`../bin/tlsh_version | tail -1 | cut -f1`
echo "HASH is $HASH"
echo "CHKSUM is $CHKSUM"

if test ! -f ../bin/tlsh
then
	echoerr "error: (127), you must compile tlsh"
        popd > /dev/null
	exit 127
fi

if test ! -f ../bin/simple_unittest
then
	echoerr "error: (127), you must compile ../bin/simple_unittest"
        popd > /dev/null
	exit 127
fi

if test ! -d tmp
then
    mkdir tmp
fi

############################
# THE runit FUNCTION
############################
#
# this function will be run twice, "" and "-xlen"
#
runit() {

	if test "$1" = "-xlen"
	then
	    XLEN="xlen"
	    echo
	    echo "Scenario: considering len, ..."
	else
	    XLEN="len"
	    echo "Scenario: not considering len, ..."
	fi
	if test "$1" = "-tlsh_c"
	then
	    TLSH_PROG="tlsh_c"
	    echo "Scenario: tlsh_c (c standalone version)..."
	else
	    TLSH_PROG="tlsh"
	    echo "Scenario: tlsh   (c++ standard version)..."
	fi

	########################################################
	# Test 1
	#	get the TLSH values for a directory of files
	########################################################

	echo
	echo "test 1"
	echo

	echo "../bin/${TLSH_PROG} -r ../Testing/example_data > $TMP/example_data.out 2> $TMP/example_data.err"
	      ../bin/${TLSH_PROG} -r ../Testing/example_data > $TMP/example_data.out 2> $TMP/example_data.err

	diffc=`diff --ignore-all-space $TMP/example_data.out exp/example_data.$HASH.$CHKSUM.$XLEN.out_EXP | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (1), diff $TMP/example_data.out exp/example_data.$HASH.$CHKSUM.$XLEN.out_EXP"
		popd > /dev/null
		exit 1
	fi
	diffc=`diff --ignore-all-space $TMP/example_data.err exp/example_data.$HASH.$CHKSUM.$XLEN.err_EXP | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (1), diff $TMP/example_data.err exp/example_data.$HASH.$CHKSUM.$XLEN.err_EXP"
		popd > /dev/null
		exit 1
	fi

	echo "passed"

	########################################################
	# Test 2
	#	calculate scores of a file (website_course_descriptors06-07.txt) compared to the directory of files
	########################################################

	echo
	echo "test 2"
	echo

	if test $XLEN = "xlen"
	then
	echo "../bin/${TLSH_PROG} -xlen -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores 2> $TMP/example_data.err2"
	      ../bin/${TLSH_PROG} -xlen -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores 2> $TMP/example_data.err2
	else
	echo "../bin/${TLSH_PROG} -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores 2> $TMP/example_data.err2"
	      ../bin/${TLSH_PROG} -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores 2> $TMP/example_data.err2
	fi

	diffc=`diff --ignore-all-space $TMP/example_data.scores exp/example_data.$HASH.$CHKSUM.$XLEN.scores_EXP | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (2), diff $TMP/example_data.scores exp/example_data.$HASH.$CHKSUM.$XLEN.scores_EXP"
		popd > /dev/null
		exit 2
	fi
	diffc=`diff --ignore-all-space $TMP/example_data.err2 exp/example_data.$HASH.$CHKSUM.$XLEN.err2_EXP | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (2), diff $TMP/example_data.err2 exp/example_data.$HASH.$CHKSUM.$XLEN.err2_EXP"
		popd > /dev/null
		exit 2
	fi

	echo "passed"

	########################################################
	# Test 3
	#	calculate scores of a file (website_course_descriptors06-07.txt) compared to hashes listed in a file
	#	far more efficient
	########################################################

	echo
	echo "test 3"
	echo

	# note that test 3 will output the following error, so write stderr to /dev/null, so it will not be seen.
	#   warning: cannot read TLSH code ../Testing/example_data/BookingBrochure.txt
	if test $XLEN = "xlen"
	then
	echo "../bin/${TLSH_PROG} -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2"
	      ../bin/${TLSH_PROG} -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2 2>/dev/null
	else
	echo "../bin/${TLSH_PROG} -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2"
	      ../bin/${TLSH_PROG} -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2 2>/dev/null
	fi

	diffc=`diff --ignore-all-space $TMP/example_data.scores.2 exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2_EXP | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (3) diff $TMP/example_data.scores.2 exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2_EXP"
		popd > /dev/null
		exit 3
	fi

	echo "passed"

	########################################################
	# Test 4
	#	Test out the -xref parameter which computes the distance scores for each file in a directory (-r parameter) with 
	#   all other files in that directory.
	########################################################
	testnum=4
	echo 
	echo "test $testnum"
	echo 
	if [ $XLEN = "xlen" ]; then
	echo "../bin/${TLSH_PROG} -xref -xlen -r ../Testing/example_data $TMP/example_data.xref.scores"
	      ../bin/${TLSH_PROG} -xref -xlen -r ../Testing/example_data > $TMP/example_data.xref.scores 2>/dev/null
	else
	echo "../bin/${TLSH_PROG} -xref -r ../Testing/example_data $TMP/example_data.xref.scores"
	      ../bin/${TLSH_PROG} -xref -r ../Testing/example_data > $TMP/example_data.xref.scores 2>/dev/null
	fi

	diff --ignore-all-space $TMP/example_data.xref.scores exp/example_data.$HASH.$CHKSUM.$XLEN.xref.scores_EXP > /dev/null 2>/dev/null
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum), diff $TMP/example_data.xref.scores exp/example_data.$HASH.$CHKSUM.$XLEN.xref.scores_EXP"
		popd > /dev/null
		exit $testnum
	fi

	echo "passed"

	########################################################
	# Test 5
	#	Test out the -T (threshold parameter)
	########################################################
	testnum=$(($testnum + 1))
	echo
	echo "test $testnum"
	echo
	# note that test 5 will output the following error, so write stderr to /dev/null, so it will not be seen.
	#   warning: cannot read TLSH code ../Testing/example_data/BookingBrochure.txt

	if [ $XLEN = "xlen" ]; then
	echo "../bin/${TLSH_PROG} -T 201 -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201"
	      ../bin/${TLSH_PROG} -T 201 -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201 2>/dev/null
	else
	echo "../bin/${TLSH_PROG} -T 201 -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201"
	      ../bin/${TLSH_PROG} -T 201 -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201 2>/dev/null
	fi

	diff --ignore-all-space $TMP/example_data.scores.2.T-201 exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2.T-201_EXP > /dev/null 2>/dev/null
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum) diff $TMP/example_data.scores.2.T-201 exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2.T-201_EXP"
		popd > /dev/null
		exit $testnum
	fi

	echo "passed"

}
############################
# END OF THE runit FUNCTION
############################

runit 
runit "-xlen"
if test -f ../bin/tlsh_c
then
	runit "-tlsh_c"
fi

############################
# Test 6
#	Test out the TLSH digest with a wide range of lengths (testlen.sh)
############################
testnum=6
echo
echo "test $testnum"
echo

# I use the papameter value of 22 for the Fibanacci sequence for generating content
# this generates files up to 6.7 Meg (good enough for automated testing)

echo "./testlen.sh 22 > $TMP/testlen.out"
      ./testlen.sh 22 > $TMP/testlen.out

diff --ignore-all-space $TMP/testlen.out exp/testlen.out_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: ($testnum) diff $TMP/testlen.out exp/testlen.out_EXP"
	popd > /dev/null
	exit $testnum
fi
echo "passed"

############################
# END OF test 6
############################

############################
# Test 7
#	Test the -force option
############################
testnum=7
echo
echo "test $testnum"
echo

for file in small small2 ; do
	echo "../bin/${TLSH_PROG} -force -f example_data/$file.txt > $TMP/$file.tlsh"
	      ../bin/${TLSH_PROG} -force -f example_data/$file.txt > $TMP/$file.tlsh

	diff --ignore-all-space $TMP/$file.tlsh exp/$file.tlsh_EXP
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum) $TMP/$file.tlsh exp/$file.tlsh_EXP"
		popd > /dev/null
		exit $testnum
	fi
done
echo "passed"

############################
# END OF test 7
############################

echo
echo "Running simple_unittest"
../bin/simple_unittest > $TMP/simple_unittest.out
diff --ignore-all-space $TMP/simple_unittest.out exp/simple_unittest_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: diff $TMP/simple_unittest.out exp/simple_unittest_EXP"
        popd > /dev/null
	exit -1
fi

echo "passed all example data tests"

popd > /dev/null

echo
echo "If you have made changes to the Tlsh python module, build and install it, and run python_test.sh"
echo
