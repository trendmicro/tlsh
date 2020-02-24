#!/bin/sh

##################################
# set CREATE_EXP_FILE=1 if you want this script to create the Expected Results for the regression tests
##################################
CREATE_EXP_FILE=0

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
cd $BASEDIR

LISTOPTION=1
JSONOPTION=1
if test "$1" = "_go"
then
	TLSH_PROG="./tlsh_go"
	SIMP_PROG="./tlsh_go"
	LISTOPTION=0
	JSONOPTION=0
	echo "Scenario: $1	(go version)..."
elif test ! -z "$1"
then
	TLSH_PROG="../bin/tlsh$1"
	SIMP_PROG="../bin/simple_unittest$1"
	echo "Scenario: $1	(c++ version)..."
else
	TLSH_PROG=../bin/tlsh
	SIMP_PROG="../bin/simple_unittest"
	echo "Scenario: tlsh	(c++ standard version)..."
fi
echo "TLSH_PROG=$TLSH_PROG"

if test ! -f $TLSH_PROG
then
	echoerr "error: (127), you must compile $TLSH_PROG"
	exit 127
fi

if test ! -f $SIMP_PROG
then
	echoerr "error: (127), you must compile $SIMP_PROG"
	exit 127
fi

TMP="tmp"
if test "$1" = "_go"
then
	HASH=128
	CHKSUM=1
	SLDWIN=5
else
	HASH=`  $TLSH_PROG -longversion | head -1           | cut -f1`
	CHKSUM=`$TLSH_PROG -longversion | head -2 | tail -1 | cut -f1`
	SLDWIN=`$TLSH_PROG -longversion |           tail -1 | cut -f1`
fi
echo "HASH is $HASH"
echo "CHKSUM is $CHKSUM"
echo "SLDWIN is $SLDWIN"

if test -d tmp
then
	rm -rf tmp
fi
mkdir tmp

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
	    echo "Scenario: not considering len, ..."
	else
	    XLEN="len"
	    echo "Scenario:     considering len, ..."
	fi
	if test "$1" = "-tlsh_c"
	then
	    TLSH_PROG="tlsh_c"
	    echo "Scenario: tlsh_c (c standalone version)..."
	fi

	########################################################
	# Test 1
	#	get the TLSH values for a directory of files
	########################################################

	echo
	echo "test 1"
	echo

	echo "${TLSH_PROG} -r example_data -o $TMP/example_data.out 2> $TMP/example_data.err"
	      ${TLSH_PROG} -r example_data -o $TMP/example_data.out 2> $TMP/example_data.err

	if test ! -f $TMP/example_data.out
	then
		echoerr "error: (1), $TMP/example_data.out does not exist"
		exit 1
	fi
	EXPECTED_OUT=exp/example_data.$HASH.$CHKSUM.$XLEN.out_EXP
	EXPECTED_ERR=exp/example_data.$HASH.$CHKSUM.$XLEN.err_EXP
	if test ! -f $EXPECTED_OUT
	then
		if test $CREATE_EXP_FILE = 0
		then
			echoerr "error: (1), Expected Result file $EXPECTED_OUT does not exist"
			exit 1
		else
			echo "cp $TMP/example_data.out $EXPECTED_OUT"
			      cp $TMP/example_data.out $EXPECTED_OUT
		fi
	fi
	if test ! -f $EXPECTED_ERR
	then
		if test $CREATE_EXP_FILE = 0
		then
			echoerr "error: (1), Expected Result file $EXPECTED_ERR does not exist"
			exit 1
		else
			echo "cp $TMP/example_data.err $EXPECTED_ERR"
			      cp $TMP/example_data.err $EXPECTED_ERR
		fi
	fi
	diffc=`diff --ignore-all-space $TMP/example_data.out $EXPECTED_OUT | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (1), diff $TMP/example_data.out $EXPECTED_OUT"
		exit 1
	fi

	diffc=`diff --ignore-all-space $TMP/example_data.err $EXPECTED_ERR | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (1), diff $TMP/example_data.err $EXPECTED_ERR"
		exit 1
	fi

	echo "passed"

	if test $JSONOPTION = 1
	then
		echo "${TLSH_PROG} -r example_data -o $TMP/example_data.json_out -ojson 2> $TMP/example_data.err"
		      ${TLSH_PROG} -r example_data -o $TMP/example_data.json_out -ojson 2> $TMP/example_data.err
		EXPECTED_OUT=exp/example_data.$HASH.$CHKSUM.$SLDWIN.$XLEN.json_out_EXP
		diffc=`diff --ignore-all-space $TMP/example_data.json_out $EXPECTED_OUT | wc -l`
		if test ! $diffc = 0
		then
			echoerr "error: (1), diff $TMP/example_data.json_out $EXPECTED_OUT"
			exit 1
		fi
		echo "passed"
	else
		echo
		echo "Test1(B): we need to implement json functionality in Go version"
	fi

	########################################################
	# Test 2
	#	calculate scores of a file (website_course_descriptors06-07.txt) compared to the directory of files
	########################################################

	echo
	echo "test 2"
	echo

	if test $XLEN = "xlen"
	then
	echo "${TLSH_PROG} -xlen -r example_data -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores 2> $TMP/example_data.err2"
	      ${TLSH_PROG} -xlen -r example_data -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores 2> $TMP/example_data.err2
	else
	echo "${TLSH_PROG} -r example_data -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores 2> $TMP/example_data.err2"
	      ${TLSH_PROG} -r example_data -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores 2> $TMP/example_data.err2
	fi

	EXPECTED_SCO=exp/example_data.$HASH.$CHKSUM.$XLEN.scores_EXP
	EXPECTED_ERR=exp/example_data.$HASH.$CHKSUM.$XLEN.err2_EXP
	if test ! -f $EXPECTED_SCO
	then
		if test $CREATE_EXP_FILE = 0
		then
			echoerr "error: (1), Expected Result file $EXPECTED_SCO does not exist"
			exit 1
		else
			echo "cp $TMP/example_data.scores $EXPECTED_SCO"
			      cp $TMP/example_data.scores $EXPECTED_SCO
		fi
	fi
	if test ! -f $EXPECTED_ERR
	then
		if test $CREATE_EXP_FILE = 0
		then
			echoerr "error: (1), Expected Result file $EXPECTED_ERR does not exist"
			exit 1
		else
			echo "cp $TMP/example_data.err2 $EXPECTED_ERR"
			      cp $TMP/example_data.err2 $EXPECTED_ERR
		fi
	fi

	diffc=`diff --ignore-all-space $TMP/example_data.scores $EXPECTED_SCO | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (2), diff $TMP/example_data.scores $EXPECTED_SCO"
		exit 2
	fi
	diffc=`diff --ignore-all-space $TMP/example_data.err2 $EXPECTED_ERR | wc -l`
	if test ! $diffc = 0
	then
		echoerr "error: (2), diff $TMP/example_data.err2 $EXPECTED_ERR"
		exit 2
	fi

	echo "passed"

	########################################################
	# Test 3
	#	calculate scores of a file (website_course_descriptors06-07.txt) compared to hashes listed in a file
	#	far more efficient
	########################################################

	if test $LISTOPTION = 1
	then
		echo
		echo "test 3"
		echo

		# note that test 3 will output the following error, so write stderr to /dev/null, so it will not be seen.
		#   warning: cannot read TLSH code example_data/BookingBrochure.txt
		if test $XLEN = "xlen"
		then
		echo "${TLSH_PROG} -xlen -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2"
		      ${TLSH_PROG} -xlen -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2 2>/dev/null
		else
		echo "${TLSH_PROG} -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2"
		      ${TLSH_PROG} -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2 2>/dev/null
		fi

		EXPECTED_SCO=exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2_EXP
		if test ! -f $EXPECTED_SCO
		then
			if test $CREATE_EXP_FILE = 0
			then
				echoerr "error: (3), Expected Result file $EXPECTED_SCO does not exist"
				exit 1
			else
				echo "cp $TMP/example_data.scores.2 $EXPECTED_SCO"
				      cp $TMP/example_data.scores.2 $EXPECTED_SCO
			fi
		fi

		diffc=`diff --ignore-all-space $TMP/example_data.scores.2 $EXPECTED_SCO | wc -l`
		if test ! $diffc = 0
		then
			echoerr "error: (3) diff $TMP/example_data.scores.2 $EXPECTED_SCO"
			exit 3
		fi

		echo "passed"
	else
		echo "Test3: we need to implement list functionality (-l/-d) in Go version"
	fi

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
	echo "${TLSH_PROG} -xref -xlen -r example_data -o $TMP/example_data.xref.scores"
	      ${TLSH_PROG} -xref -xlen -r example_data -o $TMP/example_data.xref.scores 2>/dev/null
	else
	echo "${TLSH_PROG} -xref -r example_data -o $TMP/example_data.xref.scores"
	      ${TLSH_PROG} -xref -r example_data -o $TMP/example_data.xref.scores 2>/dev/null
	fi

	EXPECTED_SCO=exp/example_data.$HASH.$CHKSUM.$XLEN.xref.scores_EXP
	if test ! -f $EXPECTED_SCO
	then
		if test $CREATE_EXP_FILE = 0
		then
			echoerr "error: ($testnum), Expected Result file $EXPECTED_SCO does not exist"
			exit 1
		else
			echo "cp $TMP/example_data.xref.scores $EXPECTED_SCO"
			      cp $TMP/example_data.xref.scores $EXPECTED_SCO
		fi
	fi

	diff --ignore-all-space $TMP/example_data.xref.scores $EXPECTED_SCO > /dev/null 2>/dev/null
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum), diff $TMP/example_data.xref.scores $EXPECTED_SCO"
		exit $testnum
	fi

	echo "passed"

	if test $JSONOPTION = 1
	then
		if [ $XLEN = "xlen" ]; then
		echo "${TLSH_PROG} -xref -xlen -r example_data -o $TMP/example_data.xref.json_scores -ojson"
		      ${TLSH_PROG} -xref -xlen -r example_data -o $TMP/example_data.xref.json_scores -ojson 2>/dev/null
		else
		echo "${TLSH_PROG} -xref -r example_data -o $TMP/example_data.xref.json_scores -ojson"
		      ${TLSH_PROG} -xref -r example_data -o $TMP/example_data.xref.json_scores -ojson 2>/dev/null
		fi
		EXPECTED_SCO=exp/example_data.$HASH.$CHKSUM.$SLDWIN.$XLEN.xref.json_scores_EXP
		diff --ignore-all-space $TMP/example_data.xref.json_scores $EXPECTED_SCO > /dev/null 2>/dev/null
		if [ $? -ne 0 ]; then
			echoerr "error: ($testnum), diff $TMP/example_data.xref.json_scores $EXPECTED_SCO"
			exit $testnum
		fi
		echo "passed"
	else
		echo
		echo "Test4(B): we need to implement json functionality in Go version"
	fi

	########################################################
	# Test 5
	#	Test out the -T (threshold parameter)
	########################################################
	if test $LISTOPTION = 1
	then
		testnum=5
		echo
		echo "test $testnum"
		echo
		# note that test 5 will output the following error, so write stderr to /dev/null, so it will not be seen.
		#   warning: cannot read TLSH code example_data/BookingBrochure.txt

		if [ $XLEN = "xlen" ]; then
		echo "${TLSH_PROG} -T 201 -xlen -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2.T-201"
		      ${TLSH_PROG} -T 201 -xlen -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2.T-201 2>/dev/null
		else
		echo "${TLSH_PROG} -T 201 -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2.T-201"
		      ${TLSH_PROG} -T 201 -l $TMP/example_data.out -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.2.T-201 2>/dev/null
		fi

		EXPECTED_SCO=exp/example_data.$HASH.$CHKSUM.$XLEN.scores.2.T-201_EXP
		if test ! -f $EXPECTED_SCO
		then
			if test $CREATE_EXP_FILE = 0
			then
				echoerr "error: ($testnum), Expected Result file $EXPECTED_SCO does not exist"
				exit 1
			else
				echo "cp $TMP/example_data.scores.2.T-201 $EXPECTED_SCO"
				      cp $TMP/example_data.scores.2.T-201 $EXPECTED_SCO
			fi
		fi

		diff --ignore-all-space $TMP/example_data.scores.2.T-201 $EXPECTED_SCO > /dev/null 2>/dev/null
		if [ $? -ne 0 ]; then
			echoerr "error: ($testnum) diff $TMP/example_data.scores.2.T-201 $EXPECTED_SCO"
			exit $testnum
		fi
		echo "passed"
	else
		echo "Test5: we need to implement list functionality (-l/-d) in Go version"
	fi

}
############################
# END OF THE runit FUNCTION
############################

runit
if test "$1" = "_go"
then
	echo "passed all example data tests (for go implementation)"
	exit 0
fi
runit "-xlen"
if test -f tlsh_c
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

echo "./testlen.sh $TLSH_PROG 22 > $TMP/testlen.out"
      ./testlen.sh $TLSH_PROG 22 > $TMP/testlen.out

EXPECTED_TESTLEN=exp/testlen.$HASH.$CHKSUM.out_EXP
if test ! -f $EXPECTED_TESTLEN
then
	if test $CREATE_EXP_FILE = 0
	then
		echoerr "error: ($testnum), Expected Result file $EXPECTED_TESTLEN does not exist"
		exit 1
	else
		echo "cp $TMP/testlen.out $EXPECTED_TESTLEN"
		      cp $TMP/testlen.out $EXPECTED_TESTLEN
	fi
fi

diff --ignore-all-space $TMP/testlen.out $EXPECTED_TESTLEN > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: ($testnum) diff $TMP/testlen.out $EXPECTED_TESTLEN"
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
	echo "${TLSH_PROG} -force -f example_data/$file.txt -o $TMP/$file.tlsh"
	      ${TLSH_PROG} -force -f example_data/$file.txt -o $TMP/$file.tlsh

	EXPECTED_TLSH=exp/$file.$HASH.$CHKSUM.tlsh_EXP
	if test ! -f $EXPECTED_TLSH
	then
		if test $CREATE_EXP_FILE = 0
		then
			echoerr "error: ($testnum), Expected Result file $EXPECTED_TLSH does not exist"
			exit 1
		else
			echo "cp $TMP/$file.tlsh $EXPECTED_TLSH"
			      cp $TMP/$file.tlsh $EXPECTED_TLSH
		fi
	fi

	diff --ignore-all-space $TMP/$file.tlsh $EXPECTED_TLSH
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum) $TMP/$file.tlsh $EXPECTED_TLSH"
		exit $testnum
	fi
done
echo "passed"

############################
# END OF test 7
############################

############################
# Test 8
#	Test the -l2 and -lcsv options
############################
testnum=8
echo
echo "test $testnum"
echo

if test $SLDWIN = 5
then
	#
	# Test 8(a): -l2
	#
	echo "${TLSH_PROG} -T 201 -l2 -l example_data_col_swap.tlsh -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.l2.T-201"
	      ${TLSH_PROG} -T 201 -l2 -l example_data_col_swap.tlsh -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.l2.T-201 2>/dev/null

	# same expected output as Test 5

	EXPECTED_SCO=exp/example_data.$HASH.$CHKSUM.len.scores.2.T-201_EXP
	if test ! -f $EXPECTED_SCO
	then
		echoerr "error: ($testnum), Expected Result file $EXPECTED_SCO does not exist"
		exit 1
	fi

	diff --ignore-all-space $TMP/example_data.scores.l2.T-201 $EXPECTED_SCO > /dev/null 2>/dev/null
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum) diff $TMP/example_data.scores.l2.T-201 $EXPECTED_SCO"
		exit $testnum
	fi

	#
	# Test 8(a): -l2 -lcsv
	#
	echo "${TLSH_PROG} -T 201 -l2 -lcsv -l example_data_col_swap.csv -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.l2csv.T-201"
	      ${TLSH_PROG} -T 201 -l2 -lcsv -l example_data_col_swap.csv -c example_data/website_course_descriptors06-07.txt -o $TMP/example_data.scores.l2csv.T-201 2>/dev/null

	# same expected output as Test 8(a) above

	diff --ignore-all-space $TMP/example_data.scores.l2csv.T-201 $EXPECTED_SCO > /dev/null 2>/dev/null
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum) diff $TMP/example_data.scores.l2csv.T-201 $EXPECTED_SCO"
		exit $testnum
	fi

	echo "passed"
else
	echo "test 8 not relevant to SLDWIN=$SLDWIN"
fi
############################
# END OF test 8
############################

############################
# Test 9
#	Test the -split option
############################
testnum=9
echo
echo "test $testnum"
echo

if test $SLDWIN = 5
then
	echo "${TLSH_PROG} -split 50,100,200 -f example_data/Week3.txt -o $TMP/example_data.Week3.split.tlsh"
	      ${TLSH_PROG} -split 50,100,200 -f example_data/Week3.txt -o $TMP/example_data.Week3.split.tlsh   2>/dev/null

	EXPECTED_RES=exp/example_data.Week3.split.tlsh
	if test ! -f $EXPECTED_RES
	then
		echoerr "error: ($testnum), Expected Result file $EXPECTED_RES does not exist"
		exit 1
	fi

	diff --ignore-all-space $TMP/example_data.Week3.split.tlsh $EXPECTED_RES > /dev/null 2>/dev/null
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum) diff $TMP/example_data.Week3.split.tlsh $EXPECTED_RES"
		exit $testnum
	fi

	echo "passed"
else
	echo "test 9 not relevant to SLDWIN=$SLDWIN"
fi

############################
# END OF test 9
############################

############################
# test 10
############################
testnum=10

echo
echo "Running $SIMP_PROG"
$SIMP_PROG > $TMP/simple_unittest.out

EXPECTED_STEST=exp/simple_unittest.$HASH.$CHKSUM.EXP
if test ! -f $EXPECTED_STEST
then
	if test $CREATE_EXP_FILE = 0
	then
		echoerr "error: ($testnum), Expected Result file $EXPECTED_STEST does not exist"
		exit 1
	else
		echo "cp $TMP/simple_unittest.out $EXPECTED_STEST"
		      cp $TMP/simple_unittest.out $EXPECTED_STEST
	fi
fi

diff --ignore-all-space $TMP/simple_unittest.out $EXPECTED_STEST > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: ($testnum) diff $TMP/simple_unittest.out $EXPECTED_STEST"
	exit 255
fi

echo "passed all example data tests"

echo
echo "If you have made changes to the Tlsh python module, build and install it, and run python_test.sh"
echo
