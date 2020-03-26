@echo off

echo cleaning TLSH

echo cd mingw
     cd mingw

echo mingw32-make clean
     mingw32-make clean

echo cd ..
     cd ..

echo del mingw\input_desc.cpp mingw\shared_file_functions.cpp mingw\tlsh.cpp mingw\ngram_block.cpp mingw\tlsh_impl.cpp
     del mingw\input_desc.cpp mingw\shared_file_functions.cpp mingw\tlsh.cpp mingw\ngram_block.cpp mingw\tlsh_impl.cpp
echo del mingw\tlsh_util.cpp mingw\tlsh_unittest.cpp mingw\simple_unittest.cpp
     del mingw\tlsh_util.cpp mingw\tlsh_unittest.cpp mingw\simple_unittest.cpp

echo del bin\tlsh.exe bin\simple_unittest.exe
     del bin\tlsh.exe bin\simple_unittest.exe

echo rmdir Testing\tmp /s /q
     rmdir Testing\tmp /s /q
