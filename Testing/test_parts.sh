#!/bin/sh

export LC_ALL='C'

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
cd $BASEDIR > /dev/null

if test ! -f ../bin/tlsh
then
	echoerr "error: (127), you must compile tlsh"
	exit 127
fi

if test ! -f ../bin/tlsh_parts
then
	echoerr "error: (127), you must compile ../bin/tlsh_parts"
	exit 127
fi

mkdir -p tmp

############################
# Test 1
############################

echo "../bin/tlsh_parts -d T11454F120A8989D5CF8CAC182F93A3E8D475C317365C5B4911C3C4A9CA5438F5E8FB6EB > out.parts"
      ../bin/tlsh_parts -d T11454F120A8989D5CF8CAC182F93A3E8D475C317365C5B4911C3C4A9CA5438F5E8FB6EB > out.parts

EXPECTED_PARTS=exp/out.parts_EXP
if test ! -f $EXPECTED_PARTS
then
	echoerr "error: Expected parts file $EXPECTED_PARTS does not exist"
	exit 1
fi

diff --ignore-all-space out.parts $EXPECTED_PARTS > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: diff out.parts $EXPECTED_PARTS"
	exit 1
fi
echo "passed"
