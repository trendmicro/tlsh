@echo off

echo compiling TLSH

if EXIST bin ( echo found dir bin ) else ( echo mkdir bin & mkdir bin)

copy src\input_desc.cpp			mingw
copy src\shared_file_functions.cpp	mingw
copy src\tlsh.cpp			mingw
copy src\ngram_block.cpp		mingw
copy src\tlsh_impl.cpp			mingw
copy src\tlsh_util.cpp			mingw
copy test\tlsh_unittest.cpp		mingw
copy test\simple_unittest.cpp		mingw

echo cd mingw
     cd mingw

echo mingw32-make
     mingw32-make

echo copy tlsh.exe ..\bin
     copy tlsh.exe ..\bin

echo copy simple_unittest.exe ..\bin
     copy simple_unittest.exe ..\bin

echo cd ..\Testing
     cd ..\Testing

CALL test

echo cd ..
     cd ..
