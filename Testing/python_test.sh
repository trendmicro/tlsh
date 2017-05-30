#!/bin/bash

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
pushd $BASEDIR > /dev/null
TMP="tmp"
PYTHON=$1

if [ -z "${PYTHON}" ]
then
    PYTHON="python"
fi

if test ! -d tmp
then
    mkdir tmp
fi

if [ -z "${CARCH}" ]
then
    CARCH=$(uname -m)
fi

PYTHONVERSION="$(${PYTHON} -c 'import sys; print("%s.%s" % (sys.version_info[0], sys.version_info[1]))')"
echo "Python version is $PYTHONVERSION"
PYTHONPATH="../py_ext/build/lib.linux-${CARCH}-${PYTHONVERSION}:${PYTHONPATH}"

echo ""
echo "Running ${PYTHON} tests for ${CARCH}-${PYTHONVERSION}"

echo
echo "testing generating hash and distance functions..."
echo
echo                    "${PYTHON} ../py_ext/test.py ../Testing/example_data/2005NISSE.txt ../Testing/example_data/1english-only.txt > $TMP/python_test.out"
PYTHONPATH=${PYTHONPATH} ${PYTHON} ../py_ext/test.py ../Testing/example_data/2005NISSE.txt ../Testing/example_data/1english-only.txt > $TMP/python_test.out
echo "diff $TMP/python_test.out exp/python_test_EXP"
      diff $TMP/python_test.out exp/python_test_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: diff $TMP/python_test.out exp/python_test_EXP"
        popd > /dev/null
	exit -1
fi

echo
echo "testing the -force option for short strings..."
echo
for file in small small2 ; do
	echo                    "${PYTHON} ../py_ext/tlsh_digest.py -force example_data/$file.txt > $TMP/py_$file.tlsh"
				 ${PYTHON} ../py_ext/tlsh_digest.py -force example_data/$file.txt > $TMP/py_$file.tlsh
	echo "diff $TMP/py_$file.tlsh exp/$file.tlsh_EXP"
	      diff $TMP/py_$file.tlsh exp/$file.tlsh_EXP
	if [ $? -ne 0 ]; then
		echoerr "error: diff $TMP/py_$file.tlsh exp/$file.tlsh_EXP"
		popd > /dev/null
		exit -1
	fi
done

echo
echo "passed"
echo 
echo "Note that if py_ext/tlshmodule.cpp has changed, then 'python setup.py build; sudo python setup.py install' must be run"

popd > /dev/null
