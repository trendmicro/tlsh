#!/bin/sh

export LC_ALL='C'

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
cd $BASEDIR

if test ! -f ../bin/tlsh
then
	echoerr "error: (127), you must compile tlsh"
	exit 127
fi

if test ! -f ../bin/tlsh_pattern
then
	echoerr "error: (127), you must compile ../bin/tlsh_pattern"
	exit 127
fi

mkdir -p tmp

############################
# Test 1
#	create a pattern file
############################
testnum=1
echo
echo "test_pattern $testnum"
echo

############
# create pattern file
#	col 1: pattern number
#	col 2: nitems in group
#	col 3: TLSH
#	col 4: radius
#	col 5: pattern label
############
PATTERN_FILE=tmp/tenfile.pat
rm -f $PATTERN_FILE
patn=0
for f in	021106_yossivassa.txt 0Alice.txt 11-17-06_Academy.txt 1english-only.txt 2005NISSE.txt			\
		2006-07_Resource_Brochure.txt 2006_2007PhysicalEducationConceptMap.txt 2007ShowcaseFilm_Package.txt	\
		22-ppa-3rd_e2snewsletter_jun06.txt  42nd_street.txt ; do

	FILE=example_data/$f
	if test ! -f $FILE
	then
		echoerr "error: (1), cannot find file $FILE"
		exit 1
	fi
	echo "../bin/tlsh -f $FILE | cut -f 1"
	tlsh=`../bin/tlsh -f $FILE | cut -f 1`
	echo "pat_$patn	1	$tlsh	30	$f"	>> $PATTERN_FILE
	patn=`expr $patn + 1`
done

EXPECTED_PATFILE=exp/tenfile.pat_EXP
if test ! -f $EXPECTED_PATFILE
then
	echoerr "error: ($testnum), Expected Pattern file $EXPECTED_PATFILE does not exist"
	exit 1
fi

diff --ignore-all-space $PATTERN_FILE $EXPECTED_PATFILE > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: ($testnum) diff $PATTERN_FILE $EXPECTED_PATFILE"
	exit $testnum
fi
echo "passed"

############################
# END OF test 1
############################

############################
# Test 2
# use tlsh_pattern
############################
testnum=2
echo
echo "test_pattern $testnum"
echo

for dir in example_data example_data_variants ; do
	RESFILE=tmp/$dir.results

	echo   "../bin/tlsh_pattern -force -r $dir -pat $PATTERN_FILE > $RESFILE"
		../bin/tlsh_pattern -force -r $dir -pat $PATTERN_FILE > $RESFILE

	EXPECTED_RESFILE=exp/$dir.results_EXP
	if test ! -f $EXPECTED_RESFILE
	then
		echoerr "error: ($testnum), Expected results file $EXPECTED_RESFILE does not exist"
		exit 1
	fi

	diff --ignore-all-space $RESFILE $EXPECTED_RESFILE > /dev/null 2>/dev/null
	if [ $? -ne 0 ]; then
		echoerr "error: ($testnum) diff $RESFILE $EXPECTED_RESFILE"
		exit $testnum
	fi
	echo "passed"
done

############################
# END OF test 2
############################
