[![Build Status](https://travis-ci.org/trendmicro/tlsh.svg?branch=master)](https://travis-ci.org/trendmicro/tlsh/)

# TLSH - Trend Micro Locality Sensitive Hash

TLSH is a fuzzy matching library.
Given a byte stream with a minimum length of 50 bytes
TLSH generates a hash value which can be used for similarity comparisons.
Similar objects will have similar hash values which allows for
the detection of similar objects by comparing their hash values.  Note that
the byte stream should have a sufficient amount of complexity.  For example,
a byte stream of identical bytes will not generate a hash value.

## Minimum byte stream length

The program in default mode requires an input byte stream with a minimum length of 256 bytes
(and a minimum amount of randomness - see note in Python extension below).

In addition the -force option allows strings down to length 50.
See notes for version 3.5.0 of TLSH

## Computed hash

The computed hash is 35 bytes long (output as 70 hexidecimal charactes). The
first 3 bytes are used to capture the information about the file as a whole
(length, ...), while the last 32 bytes are used to capture information about
incremental parts of the file.  (Note that the length of the hash can be
increased by changing build parameters described below in [CMakeLists.txt](CMakeLists.txt),
which will increase the information stored in the hash.
For some applications this might increase the accuracy in predicting similarities between files.)

## Executables and library

Building TLSH (see below) will create a static library in the `lib` directory,
and the `tlsh` executable (a symbolic link to `tlsh_unittest`).
'tlsh' links to the static library, in the `bin` directory.
The library has functionality to generate the hash value from a given
file, and to compute the similarity between two hash values.

`tlsh` is a utility for generating TLSH hash values and comparing TLSH
hash values to determine similarity.  Run it with no parameters for detailed usage.

## Ports

- A JavaScript port available in the `js_ext` directory.
- A Java port is available in the `java` directory.
- Another Java port is available [here](https://github.com/idealista/tlsh).
- A Golang port is available [here](https://github.com/glaslos/tlsh).
- A Ruby port is available [here](https://github.com/adamliesko/tlsh)

# Downloading TLSH

Download TLSH as follows:

```
wget https://github.com/trendmicro/tlsh/archive/master.zip -O master.zip
unzip master.zip
cd tlsh-master
```

**or**

```
git clone git://github.com/trendmicro/tlsh.git
cd tlsh
git checkout master
```

# Building TLSH

Edit [CMakeLists.txt](CMakeLists.txt) to build TLSH with different options.

- TLSH_BUCKETS: determines using 128 or 256 buckets
	use the default 128 buckets unless you are an expert and know you need 256 buckets
- TLSH_CHECKSUM_1B: determines checksum length, longer means less collision
	use the default 1 byte unless you are an expert and know you need a larger checksum

## Linux

Execute:

```
make.sh
```

**Note:** *Building TLSH on Linux depends upon `cmake` to create the `Makefile` and then
`make` the project, so the build will fail if `cmake` is not installed.*


## Windows (Visual Studio)

Use the version-specific tlsh solution files ([tlsh.VC2005.sln](Windows/tlsh.VC2005.sln),
[tlsh.VC2008.sln](Windows/tlsh.VC2008.sln), ...) under the Windows directory.

See [tlsh.h](include/tlsh.h) for the tlsh library interface and [tlsh_unittest.cpp](test/tlsh_unittest.cpp) and
[simple_unittest.cpp](test/simple_unittest.cpp) under the `test` directory for example code.

## Python Extension

There is a README.python with notes about the python version

```
(1) compile the C++ code
	$./make.sh
(2) build the python version
	$ cd py_ext/
	$ python ./setup.py build
(3) install - possibly - sudo, run as root or administrator
	$ python ./setup.py install
(4) test it
	$ cd ../Testing
	$ ./python_test.sh
```

### Python API

```python
import tlsh
tlsh.hash(data)
```


Note that in default mode the data must contain at least 256 bytes to generate a hash value and that
it must have a certain amount of randomness.
If you use the "force" option, then the data must contain at least 50 characters.
For example, `tlsh.hash(str(os.urandom(256)))`, should always generate a hash.  
To get the hash value of a file, try `tlsh.hash(open(file, 'rb').read())`.

```python
tlsh.diff(h1, h2)
tlsh.diffxlen(h1, h2)
```

The `diffxlen` function removes the file length component of the tlsh header from
the comparison.  If a file with a repeating pattern is compared to a file
with only a single instance of the pattern, then the difference will be increased
if the file lenght is included.  But by using the `diffxlen` function, the file
length will be removed from consideration.

Note that the python API has been extended to miror the C++ API.  See
py_ext/tlshmodule.cpp and the py_ext/test.py script to see the full API set.

# Design Choices

- To improve comparison accuracy, TLSH tracks counting bucket height
  distribution in quartiles. Bigger quartile difference results in higher
  difference score.
- Use specially 6 trigrams to give equal representation of the bytes in the 5
  byte sliding window which produces improved results.
- Pearson hash is used to distribute the trigram counts to the counting buckets.
- The global similarity score distances objects with significant size
  difference. Global similarity can be disabled. It also distances objects with
  different quartile distributions.
- TLSH can be compiled to generate 70 or 134 characters hash strings.
  The longer version has been created to use of the 70 char hash strings is not working
  for your application.

TLSH similarity is expressed as a difference score:

- A score of 0 means the objects are almost identical.
- For the 70 characters hash, there is a detailed table of experimental Detection rates and False Positive rates
  based on the threshhold. see [Table II on page 5](https://github.com/trendmicro/tlsh/blob/master/TLSH_CTC_final.pdf)

# Publications

- Jonathan Oliver, Chun Cheng, and Yanggui Chen, [TLSH - A Locality Sensitive Hash](https://github.com/trendmicro/tlsh/blob/master/TLSH_CTC_final.pdf).
4th Cybercrime and Trustworthy Computing Workshop, Sydney, November 2013
- Jonathan Oliver, Scott Forman, and Chun Cheng, [Using Randomization to Attack Similarity Digests](https://github.com/trendmicro/tlsh/blob/master/Attacking_LSH_and_Sim_Dig.pdf).
ATIS 2014, November, 2014, pages 199-210

# Changes

**3.0.0**
- Implemented TLSH.
- Updated to build with CMake.

**3.0.1**
- Enabled C++ optimization. Runs 4x faster.

**3.0.2**
- Supports Windows and Visual Studio.

**3.0.3**
- Added Python extension library. TLSH is callable in Python.
- Stop generating hash if the input is less than 512 bytes.
- Cleaned up.

**3.0.4**
- Length difference consideration can be disabled in this version. See `totalDiff` in `tlsh.h`.
- TLSH can be compiled to generate the 70 or 134 character hashes. The longer version is more accurate.

**3.1.0**
- The checksum can be changed from 1 byte to 3 bytes. The collison rate is lower using 3 bytes.
- If the incoming data has few features. The algorithm will not generate hash value. At least half the buckets must be non-zero.
- Null or invalid hash strings comparison will return `-EINVAL` (-22).
- Python extension library will read `CMakeLists.txt` to pick the compile options.
- The default build will use half the buckets and 1 byte checksum.
- New executable `tlsh_version` reports number of buckets, checksum length.

**3.1.1**
- Add `make.sh` and `clean.sh` scripts for building/cleaning the project.
- Modifications to `tlsh_unittest.cpp` to write errors to stderr (not stdout) and to continue processing in some error cases. Also handle a listfile (`-l` parameter) which contains both TLSH and filename.
- Updated expected output files based on changes to `tlsh_unittest.cpp`.

**3.1.2**
- Updated the Testing/exp expected results.
- Created a script to ease the creation of the Testing/exp expected results.

**3.1.3**
- Updated `tlsh_util.h`, `tlsh_impl.cpp`, `tlsh_util.cpp` on checksum.
- Updated `destroy_refersh_exp.sh` and Testing/exp results.

**3.2.0**
- Add Visual Studio 2005 and 2008 project and solution files to enable build on Windows environment.
- Added files `WinFunctions.h` and `WinFunctions.cpp` to handle code changes needed for Windows build.
- Modified several unit test expected output files to remove error messages, to allow the running of unit tests on Windows under Cygwin.  This was caused by the opposite order in which stdout and stderr are written when stderr is redirected to stdout as 2>&1.  Also modified `test.sh` to write stderr to `/dev/null`.
- Move `rand_tags` executable from tlsh_forest project to tlsh, to reduce the dependencies of the tlsh ROC analysis project, which depends upon `tlsh_unittest` and `rand_tags`.
- Remove `simple_unittest` and `tlsh_version` from bin directory as these executables are for internal testing and source code documentation, and do not need to be exported.  
- Add -version flag to `tlsh_unittest` to get the version of the tlsh library.

**3.2.1**
- Pickup fix to `hash_py()` in `py_ext/tlshmodule.cpp` (commit da5370bcfdd40dd6a33c877ee87fe3866188cf2d).

**3.3.0**
- Made the minimum data length = 256 for the C version.
	This was reduced in version 3.5.0 to 50 bytes (the force option)

**3.3.1**
- Fixed bug introduced by commit 1a8f1c581c8b988ced683ff8e0a0f9c574058df4 which caused a different hash value to be generated if there were multiple calls to `Tlsh::update` as opposed to a single call.

**3.4.0**
- Add JavaScript implementation (see directory `js_ext`) - required for [Blackhat presentation](https://www.blackhat.com/us-15/speakers/Sean-Park.html).
- Modify `tlsh_unittest` so that it can output tlsh values and filenames correctly, when the filenames contain embedded newline, linefeed or tab characters.

**3.4.1**
- Thanks to Jeremy Bobbios `py_ext` patch. TLSH has these enhancements.
- Instead of using a big memory blob, it will calculate the hash incrementally.
- A hashlib like object-oriented interface has been added to the Python module. See `test.py`.
- Restrict the function to be fed bytes-like object to remove surprises like silent UTF-8 decoding.

**3.4.2**
- Back out python regression test as part of the test.sh script, so that the python module does not need to be installed in order to successfully pass the tests run by make.sh

**3.4.3**
- Fix regression tests running on Windows

**3.4.4**
- Specify Tlsh::getHash() is a const method

**3.4.5**
- Pick up Jeremy Bobbios patches for:
  - Build shared library (libtlsh.so), in addition to static library, on Linux and have tlsh_unittest link to it.
  - Remove TlshImpl symbols from libtlsh.so
  - Add Tlsh_init to py_ext/tlshmodule.cpp, which ensures Tlsh constructor will be called from Tlsh python module
  - Create symbolic link for tlsh -> tlsh_unittest

**3.5.0**
- Added the - force option
  - Allows a user to force the generation of digests for strings down to 50 characters long

**3.5.1**
- Fixed the error in the Python extension

**3.5.2**
- Added the BlackHat Asia tool (presented at Arsenal)

**3.6.0**
- skipped

**3.7.0**
- merged in various fixes - ifdef for SPARC and RH73
  corrected TLSH_CTC_final.pdf (see https://github.com/trendmicro/tlsh/issues/31)
  added a SHA1 to the NOTICE.txt file
  improved the make.sh so that it calls the test.sh (and does regression tests)
  improved regression tests to confirm that the hash is calculated correctly in your environment
  fixed the header file C++ standard violation (reserved identifier violation #21)

**3.7.1**
- resolved issue #29 - the force option for Python
  Step 1 - adding a regression test for strings approx of length 50
  Step 2 - add python code

**3.7.2**
- added code to set the distance parameters for ROC analysis
to use these settings then change in CMakeLists.txt
set(TLSH_DISTANCE_PARAMETERS 0)
=>
set(TLSH_DISTANCE_PARAMETERS 1)

**3.7.3**
- resolving issue #44
- making static library the default

**3.7.4**
- resolving issue #45
- add a timing test for TLSH
<PRE>
  $ bin/timing_unittest
  build a buffer with a million bytes...
  eval TLSH 50 times...
  TLSH(buffer) = A12500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
  BEFORE	ms=1502905567631
  AFTER	ms=1502905573523
  TIME	ms=5892
  TIME	ms=117	per iteration
</PRE>

**3.7.5**
- resolving issue #46
- in include/tlsh_impl.h
	#define SLIDING_WND_SIZE  5
this can be varied between 4 to 8

**3.8.0**
Adding    // access functions - required by tools using TLSH library
+    int Lvalue();
+    int Q1ratio();
+    int Q2ratio();

**3.9.0**
resolving issue #48 - tlsh_pattern program
This tlsh_pattern program should read a pattern file
+ col 1: pattern number
+ col 2: nitems in group
+ col 3: TLSH
+ col 4: radius
+ col 5: pattern label
The input options should match the tlsh program
<PRE>
usage: tlsh_pattern [-xlen] [-force] -pat pattern_file -f file
: tlsh_pattern [-xlen] [-force] -pat pattern_file -d digest
: tlsh_pattern [-xlen] [-force] -pat pattern_file -r dir
: tlsh_pattern [-xlen] [-force] -pat pattern_file -l listfile
</PRE>

**3.9.1**
resolving issue #38
putting in fix in rand_tags.cpp so that it generates identical output to previous version
while safely working with pointers

**3.9.2**
<PRE>
18/Mar/2019
Also merged the contents of NOTICE.txt into LICENSE.
This was done because NOTICE.txt is sometimes accidently removed when people clone this repository.
And the LICENSE specifically states that NOTICE.txt should NOT be removed.

Also added command line option -notice which displays the NOTICE.txt file
</PRE>

**3.9.3**
<PRE>
19/Mar/2019
currently tlsh_pattern returns all the matches
modify tlsh_pattern to return the best match

remove the newline from the input fields when reading in the tlsh_pattern file
</PRE>

**3.9.4**
<PRE>
19/Mar/2019
check in order_bug program which demonstrates issue #50
resolved issue #50 - added code to tlsh_impl.cpp to check for invalid call sequences to update() and final()
</PRE>

**3.9.5**
<PRE>
19/Mar/2019
issue #61: added a command line option -notest - do not do any testing
	./make.sh -notest
</PRE>

**3.9.6**
<PRE>
19/Mar/2019
Have a cmake option to build tlsh with a zero byte checksum (development / research option)
Default build has 1 byte checksum - which is strongly recommended
</PRE>

**3.9.7**
<PRE>
19/Mar/2019
resolving issue #50 for bin/timing_unittest
</PRE>

**3.9.8**
<PRE>
19/Mar/2019
timing_unittest measures the time taken to do distance calculations
add a command line option -size - so that you can measure the time taken to evaluate different sizes of string
</PRE>

**3.9.9**
<PRE>
19/Mar/2019
resolve issue #62
remove dependancy on GNUInstallDirs
</PRE>

**3.10.0**
<PRE>
19/Mar/2019
Adding // access function - required by tools using TLSH library
	int BucketValue(int bucket);
	int Checksum(int k);
</PRE>

**3.11.0**
<PRE>
19/Mar/2019
Make calculation of TLSH digests approx 7 times faster (for large files)
done by
	- inline functions
	- unrolling loops
	- fixing the -O2 optimization option

<H3>Timing on Amazon linux</H3>
#       "Amazon Linux 2 AMI (HVM), SSD Volume Type"
#       Description: Amazon Linux 2 comes with five years support. It provides Linux kernel 4.14 tuned for optimal performance
#               on Amazon EC2, systemd 219, GCC 7.3, Glibc 2.26, Binutils 2.29.1, and the latest software packages through extras.

BEFORE
$ ./tlsh_3_10_0/bin/timing_unittest 
build a buffer with a million bytes...
eval TLSH (3.9.9 compact hash 1 byte checksum sliding_window=5) 50 times...
TLSH(buffer) = A12500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
Test 1: Evaluate TLSH digest
BEFORE	ms=1552963428277
AFTER	ms=1552963433258
TIME	ms=4981
TIME	ms=99	per iteration

eval TLSH distance 50 million times...
Test 2: Calc distance TLSH digest
dist=138
BEFORE	ms=1552963433362
AFTER	ms=1552963440723
TIME	ms=7361
TIME	ms=147	per million iterations
 
AFTER
$ ./tlsh_3_11_0/bin/timing_unittest 
build a buffer with a million bytes...
eval TLSH (3.11.0 compact hash 1 byte checksum sliding_window=5) 50 times...
TLSH(buffer) = A12500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
Test 1: Evaluate TLSH digest
BEFORE	ms=1552963419037
AFTER	ms=1552963419628
TIME	ms=591
TIME	ms=11	per iteration

eval TLSH distance 50 million times...
Test 2: Calc distance TLSH digest
dist=138
BEFORE	ms=1552963419642
AFTER	ms=1552963421519
TIME	ms=1877
TIME	ms=37	per million iterations


<H3>Timing on a Mac (Processor 3.1 GHz - running Sierra)</H3>

BEFORE using tlsh_3_10_0
$ bin/timing_unittest 
build a buffer with a million bytes...
eval TLSH (3.10.0 compact hash 1 byte checksum sliding_window=5) 50 times...
TLSH(buffer) = A12500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
Test 1: Evaluate TLSH digest
BEFORE	ms=1552963383885
AFTER	ms=1552963387866
TIME	ms=3981
TIME	ms=79	per iteration

eval TLSH distance 50 million times...
Test 2: Calc distance TLSH digest
dist=138
BEFORE	ms=1552963387951
AFTER	ms=1552963392498
TIME	ms=4547
TIME	ms=90	per million iterations

AFTER using tlsh_3_11_0
$ bin/timing_unittest 
build a buffer with a million bytes...
eval TLSH (3.11.0 compact hash 1 byte checksum sliding_window=5) 50 times...
TLSH(buffer) = A12500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
Test 1: Evaluate TLSH digest
BEFORE	ms=1552963360177
AFTER	ms=1552963360791
TIME	ms=614
TIME	ms=12	per iteration

eval TLSH distance 50 million times...
Test 2: Calc distance TLSH digest
dist=138
BEFORE	ms=1552963360808
AFTER	ms=1552963365502
TIME	ms=4694
TIME	ms=93	per million iterations
</PRE>

**3.11.1**
<PRE>
31/May/2019
tidy up:
1. use fast_b_mapping() instead of b_mapping()
2. remove declaration of unsigned r which is never used
3. remove #include which is not required
</PRE>

**3.12.0**
<PRE>
31/May/2019
remove floating point calculations such as log() function
use alookup table instead
</PRE>

**3.13.0**
<PRE>
31/May/2019
.vcproj files and instructions for builing TLSH on Windows using Visual Studio
Thanks Jayson Pryde! :-)
</PRE>

**3.13.1**
<PRE>
31/May/2019
fixing setup.py so that you can install Python Extension on Windows
</PRE>

**3.14.0**
<PRE>
18/July/2019
adding sliding window size to tlsh_version
changing test.sh to read the sliding window size
</PRE>

**3.14.1**
<PRE>
18/July/2019
fixing error in test script for -xlen option (print statements about considering length were incorrect)
improved test.sh - tests for existance of expected output files
</PRE>

**3.15.0**
<PRE>
19/July/2019
Refactor code - so that input of directory or digest is in a struct.
The code to process input is in library code (input_desc.cpp, shared_file_functions.cpp).
The input routines can be used by myultiple programs.
Also, preparing for things like csv input files.
</PRE>

**3.15.1**
<PRE>
19/July/2019
added command line option -help to show full help information
</PRE>

**3.15.2**
<PRE>
19/July/2019
tlsh_pattrern uses refactored code introduced in 3.15.0
</PRE>

**3.16.0**
<PRE>
19/July/2019
improved tlsh_pattern functionality
usage: tlsh_pattern -f <file>                     [-showmiss T] -pat <pattern_file> [-xlen] [-force]
     : tlsh_pattern -d <digest>                   [-showmiss T] -pat <pattern_file> [-xlen] [-force]
     : tlsh_pattern -r <dir>                      [-showmiss T] -pat <pattern_file> [-xlen] [-force]
     : tlsh_pattern -l <listfile> [-l1|-l2|-lcsv] [-showmiss T] -pat <pattern_file> [-xlen] [-force]
     : tlsh_pattern -version: prints version of tlsh library
add options
- to have different columns of a listfile be processed (-l1 or -l2)
- to allow a listfile to be in CSV format (-lcsv)
- to show misses up to threshold T (-showmiss T)
added regression tests for tlsh_pattern
</PRE>

**3.16.1**
<PRE>
19/July/2019
improved tlsh functionality
add options
  -out_fname:         Specifies that only the filename is outputted when using the -r option (no path included in output)
  -out_dirname        Specifies that the dirname and filename are outputted when using the -r option (no path included in output)

  -l1                 (default) listfile contains TLSH value in column 1
  -l2                           listfile contains TLSH value in column 2
  -lcsv               listfile is csv (comma seperated) file (default is TAB seperated file)

  -split linenumbers: linenumbers is a comma seperated list of line numbers (example 50,100,200 )
                      split the file into components and eval the TLSH for each component
                      example. -split 50,100,200 evals 4 TLSH digests. lines 1-49, 50-99, 100-199, 200-end
                      for the purpose of splitting the file, each line has a max length of 2048 bytes
</PRE>

**3.16.2**
<PRE>
19/July/2019
added regression tests for 3.16.1
by adding tests for -split, swapping columns in input files, and for CVS input file
</PRE>

**3.17.0**
<PRE>
19/July/2019
Make command line option	-force		(50 character limit) the default behaviour
Add a command line option	-conservative	(256 character limit)
change the force_option parameter to be a bit field
	force_option	==	0	Default (50 char limit)
	force_option	==	1	Force behaviour (50 char limit)
	force_option	==	2	conservative behaviour (256 char limit)
</PRE>
