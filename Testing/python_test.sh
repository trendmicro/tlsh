#!/bin/sh

echoerr() { echo "$@" 1>&2; }

BASEDIR=$(dirname $0)
cd $BASEDIR
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
	exit 255
fi
echo "pass"
echo
echo "testing the -old and -conservative options for short strings..."
echo
for file in small small2 ; do
	echo                    "${PYTHON} ../py_ext/tlsh_digest.py example_data/$file.txt > $TMP/py_$file.tlsh"
				 ${PYTHON} ../py_ext/tlsh_digest.py example_data/$file.txt > $TMP/py_$file.tlsh
	echo "diff $TMP/py_$file.tlsh exp/$file.128.1.tlsh_EXP"
	      diff $TMP/py_$file.tlsh exp/$file.128.1.tlsh_EXP
	if [ $? -ne 0 ]; then
		echoerr "error: diff $TMP/py_$file.tlsh exp/$file.128.1.tlsh_EXP"
		exit 255
	fi

	echo                    "${PYTHON} ../py_ext/tlsh_digest.py -old example_data/$file.txt > $TMP/py_$file.old.tlsh"
				 ${PYTHON} ../py_ext/tlsh_digest.py -old example_data/$file.txt > $TMP/py_$file.old.tlsh
	echo "diff $TMP/py_$file.old.tlsh exp/$file.128.1.old.tlsh_EXP"
	      diff $TMP/py_$file.old.tlsh exp/$file.128.1.old.tlsh_EXP
	if [ $? -ne 0 ]; then
		echoerr "error: diff $TMP/py_$file.old.tlsh exp/$file.128.1.old.tlsh_EXP"
		exit 255
	fi

	echo                    "${PYTHON} ../py_ext/tlsh_digest.py -conservative example_data/$file.txt > $TMP/py_$file.cons.tlsh"
				 ${PYTHON} ../py_ext/tlsh_digest.py -conservative example_data/$file.txt > $TMP/py_$file.cons.tlsh
	echo "diff $TMP/py_$file.cons.tlsh exp/$file.128.1.cons.tlsh_EXP"
	      diff $TMP/py_$file.cons.tlsh exp/$file.128.1.cons.tlsh_EXP
	if [ $? -ne 0 ]; then
		echoerr "error: diff $TMP/py_$file.cons.tlsh exp/$file.128.1.cons.tlsh_EXP"
		exit 255
	fi
done

echo
echo "testing lvalue, q1ratio, q2ratio, checksum, bucket_value function (added in 4.7.0) ..."
echo
echo                    "${PYTHON} ../py_ext/tlsh_parts.py -d T11454F120A8989D5CF8CAC182F93A3E8D475C317365C5B4911C3C4A9CA5438F5E8FB6EB > $TMP/python_parts_test.out"
PYTHONPATH=${PYTHONPATH} ${PYTHON} ../py_ext/tlsh_parts.py -d T11454F120A8989D5CF8CAC182F93A3E8D475C317365C5B4911C3C4A9CA5438F5E8FB6EB > $TMP/python_parts_test.out
echo "diff $TMP/python_parts_test.out exp/python_parts_test_EXP"
      diff $TMP/python_parts_test.out exp/python_parts_test_EXP > /dev/null 2>/dev/null
if [ $? -ne 0 ]; then
	echoerr "error: diff $TMP/python_parts_test.out exp/python_parts_test_EXP"
	exit 255
fi
echo "pass"

echo
echo "passed"
echo 
### echo "Note that if py_ext/tlshmodule.cpp has changed, then 'python setup.py build; sudo python setup.py install' must be run"

