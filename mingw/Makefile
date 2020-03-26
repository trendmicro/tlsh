
INCLUDES= -I../include

CFLAGS=$(INCLUDES) -DMINGW

TARGETS=tlsh.exe simple_unittest.exe
OBJECTS=input_desc.o shared_file_functions.o tlsh.o tlsh_impl.o tlsh_util.o

all: $(TARGETS)

clean:
	del *.o $(TARGETS)

tlsh.exe: $(OBJECTS) tlsh_unittest.o
	g++ -o tlsh.exe $(OBJECTS) tlsh_unittest.o -static-libgcc -static-libstdc++

simple_unittest.exe: $(OBJECTS) simple_unittest.o
	g++ -o simple_unittest.exe $(OBJECTS) simple_unittest.o -static-libgcc -static-libstdc++

.cpp.o:
	g++ -c $(CFLAGS) $*.cpp
