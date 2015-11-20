#!/bin/bash

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
pushd $BASEDIR > /dev/null
TMP="tmp"

if test ! -d tmp
then
    mkdir tmp
fi

echo ""
echo "Running python tests"
python ../py_ext/test.py ../Testing/example_data/2005NISSE.txt ../Testing/example_data/1english-only.txt > $TMP/python_test.out
diff --ignore-all-space $TMP/python_test.out exp/python_test_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: diff $TMP/python_test.out exp/python_test_EXP"
        popd > /dev/null
	exit -1
fi


echo "passed"
echo 
echo "Note that if py_ext/tlshmodule.cpp has changed, then 'python setup.exe build; sudo python setup.exe install' must be run"

popd > /dev/null
