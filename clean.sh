#!/bin/sh

echo "rm -rf bin build lib Testing/tmp Testing/out.parts test/simple_unittest test/tlsh_unittest VERSION include/tlsh_version.h"
      rm -rf bin build lib Testing/tmp Testing/out.parts test/simple_unittest test/tlsh_unittest VERSION include/tlsh_version.h

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

echo rm -f mingw/input_desc.cpp mingw/shared_file_functions.cpp mingw/tlsh.cpp mingw/ngram_block.cpp mingw/tlsh_impl.cpp
     rm -f mingw/input_desc.cpp mingw/shared_file_functions.cpp mingw/tlsh.cpp mingw/ngram_block.cpp mingw/tlsh_impl.cpp
echo rm -f mingw/tlsh_util.cpp mingw/tlsh_unittest.cpp mingw/simple_unittest.cpp
     rm -f mingw/tlsh_util.cpp mingw/tlsh_unittest.cpp mingw/simple_unittest.cpp
echo rm -f mingw/tlsh.exe mingw/simple_unittest.exe mingw/*.o
     rm -f mingw/tlsh.exe mingw/simple_unittest.exe mingw/*.o
echo rm -f bin/tlsh.exe bin/simple_unittest.exe
     rm -f bin/tlsh.exe bin/simple_unittest.exe
