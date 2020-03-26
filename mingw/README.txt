To make tlsh.exe using MinGW, use the command "mingw32-make"

C:\mingw> mingw32-make

g++ -c -I../include -DMINGW input_desc.cpp
g++ -c -I../include -DMINGW shared_file_functions.cpp
g++ -c -I../include -DMINGW tlsh.cpp
g++ -c -I../include -DMINGW tlsh_impl.cpp
g++ -c -I../include -DMINGW tlsh_util.cpp
g++ -c -I../include -DMINGW tlsh_unittest.cpp
g++ -o tlsh.exe input_desc.o shared_file_functions.o tlsh.o tlsh_impl.o tlsh_util.o tlsh_unittest.o
g++ -c -I../include -DMINGW simple_unittest.cpp
g++ -o simple_unittest.exe input_desc.o shared_file_functions.o tlsh.o tlsh_impl.o tlsh_util.o simple_unittest.o
