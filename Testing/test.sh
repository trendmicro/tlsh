#!/bin/bash

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
pushd $BASEDIR > /dev/null

TMP="tmp"
HASH=`../bin/tlsh_version | head -1 | cut -f1`
CHKSUM=`../bin/tlsh_version | tail -1 | cut -f1`
echo "HASH is $HASH"
echo "CHKSUM is $CHKSUM"

if test ! -f ../bin/tlsh_unittest
then
	echoerr "error: (127), you must compile tlsh_unittest"
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

#
# this function will be run twice, "" and "-xlen"
#
runit() {

if test "$1" = "-xlen"
then
    XLEN="xlen"
    echo
    echo "Running, considering len, ..."
else
    XLEN="len"
    echo "Running, not considering len, ..."
fi  

########################################################
# Test 1
#	get the TLSH values for a directory of files
########################################################

echo
echo "test 1"
echo

echo "../bin/tlsh_unittest -r ../Testing/example_data > $TMP/example_data.out"
      ../bin/tlsh_unittest -r ../Testing/example_data > $TMP/example_data.out

diffc=`diff --ignore-all-space $TMP/example_data.out exp/example_data.$HASH.$CHKSUM.$XLEN.out_EXP | wc -l`
if test ! $diffc = 0
then
	echoerr "error: (1), diff $TMP/example_data.out exp/example_data.$HASH.$CHKSUM.$XLEN.out_EXP"
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
echo "../bin/tlsh_unittest -xlen -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores"
      ../bin/tlsh_unittest -xlen -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores
else
echo "../bin/tlsh_unittest -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores"
      ../bin/tlsh_unittest -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores
fi

diffc=`diff --ignore-all-space $TMP/example_data.scores exp/example_data.$HASH.$CHKSUM.$XLEN.scores_EXP | wc -l`
if test ! $diffc = 0
then
	echoerr "error: (2), diff $TMP/example_data.scores exp/example_data.$HASH.$CHKSUM.$XLEN.scores_EXP"
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
echo "../bin/tlsh_unittest -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2"
      ../bin/tlsh_unittest -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2 2>/dev/null
else
echo "../bin/tlsh_unittest -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2"
      ../bin/tlsh_unittest -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2 2>/dev/null
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
echo "../bin/tlsh_unittest -xref -xlen -r ../Testing/example_data $TMP/example_data.xref.scores"
      ../bin/tlsh_unittest -xref -xlen -r ../Testing/example_data > $TMP/example_data.xref.scores 2>/dev/null
else
echo "../bin/tlsh_unittest -xref -r ../Testing/example_data $TMP/example_data.xref.scores"
      ../bin/tlsh_unittest -xref -r ../Testing/example_data > $TMP/example_data.xref.scores 2>/dev/null
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
echo "../bin/tlsh_unittest -T 201 -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201"
      ../bin/tlsh_unittest -T 201 -xlen -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201 2>/dev/null
else
echo "../bin/tlsh_unittest -T 201 -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201"
      ../bin/tlsh_unittest -T 201 -l $TMP/example_data.out -c ../Testing/example_data/website_course_descriptors06-07.txt > $TMP/example_data.scores.2.T-201 2>/dev/null
fi

diff --ignore-all-space $TMP/example_data.scores.2.T-201 exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2.T-201_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: ($testnum) diff $TMP/example_data.scores.2.T-201 exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2.T-201_EXP"
        popd > /dev/null
	exit $testnum
fi

echo "passed"

}

runit 
runit "-xlen"

echo
echo "Running simple_unittest"
../bin/simple_unittest > $TMP/simple_unittest.out
diff --ignore-all-space $TMP/simple_unittest.out exp/simple_unittest_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: diff $TMP/simple_unittest.out exp/simple_unittest_EXP"
        popd > /dev/null
	exit -1
fi

echo "passed"

popd > /dev/null

echo
echo "If you have made changes to the Tlsh python module, build and install it, and run python_test.sh"
echo
