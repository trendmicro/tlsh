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
PYTHONPATH=${PYTHONPATH} ${PYTHON} ../py_ext/test.py ../Testing/example_data/2005NISSE.txt ../Testing/example_data/1english-only.txt > $TMP/python_test.out
diff --ignore-all-space $TMP/python_test.out exp/python_test_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: diff $TMP/python_test.out exp/python_test_EXP"
        popd > /dev/null
	exit -1
fi


echo "passed"
echo 
echo "Note that if py_ext/tlshmodule.cpp has changed, then 'python setup.py build; sudo python setup.py install' must be run"

popd > /dev/null
