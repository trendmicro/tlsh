#!/bin/bash

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
pushd $BASEDIR > /dev/null

if test ! -f ../bin/tlsh_unittest
then
	echoerr "error: (127), you must compile tlsh_unittest"
        popd > /dev/null
	exit 127
fi

if test ! -d tmp
then
    mkdir tmp
fi

########################################################
# Test 1
#	get the TLSH values for a directory of files
########################################################

echo
echo "test 1"
echo

echo "../bin/tlsh_unittest -r ../Testing/example_data > tmp/example_data.out"
      ../bin/tlsh_unittest -r ../Testing/example_data > tmp/example_data.out

diffc=`diff --ignore-all-space tmp/example_data.out exp_long/example_data.out_EXP | wc -l`
if test ! $diffc = 0
then
	echoerr "error: (1), diff tmp/example_data.out exp_long/example_data.out_EXP"
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

echo "../bin/tlsh_unittest -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > tmp/example_data.scores"
      ../bin/tlsh_unittest -r ../Testing/example_data -c ../Testing/example_data/website_course_descriptors06-07.txt > tmp/example_data.scores

diffc=`diff --ignore-all-space tmp/example_data.scores exp_long/example_data.scores_EXP | wc -l`
if test ! $diffc = 0
then
	echoerr "error: (2), diff tmp/example_data.scores exp_long/example_data.scores_EXP"
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

echo "cut -f 1 tmp/example_data.out > tmp/example_data.tlsh"
      cut -f 1 tmp/example_data.out > tmp/example_data.tlsh
echo "../bin/tlsh_unittest -l tmp/example_data.tlsh -c ../Testing/example_data/website_course_descriptors06-07.txt > tmp/example_data.scores.2"
      ../bin/tlsh_unittest -l tmp/example_data.tlsh -c ../Testing/example_data/website_course_descriptors06-07.txt > tmp/example_data.scores.2

diffc=`diff --ignore-all-space tmp/example_data.scores.2 exp_long/example_data.scores.2_EXP | wc -l`
if test ! $diffc = 0
then
	echoerr "error: (3) diff tmp/example_data.scores.2 exp_long/example_data.scores.2_EXP"
        popd > /dev/null
	exit 3
fi

echo "passed"
popd > /dev/null
