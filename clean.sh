#!/bin/sh

echo "rm -rf bin build lib Testing/tmp test/simple_unittest test/tlsh_version test/tlsh_unittest VERSION include/version.h"
      rm -rf bin build lib Testing/tmp test/simple_unittest test/tlsh_version test/tlsh_unittest VERSION include/version.h

echo "rm -rf py_ext/build"
      rm -rf py_ext/build

if test -d src_c_standalone
then
	echo "cd src_c_standalone"
	      cd src_c_standalone
	make clean
	echo "cd .."
	      cd ..
fi
