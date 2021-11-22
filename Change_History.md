# Change History

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

**3.17.3**
<PRE>
24/March/2020
add checking to confirm that TLSH digests are the correct length in
	-c option
	-d option
	the appropriate column of -l listfile options
</PRE>

**3.18.0**
<PRE>
24/March/2020
resolve issue #72 - remove tlsh_version
</PRE>

**3.19.0**
<PRE>
24/March/2020
preperation for Windows build
- remove ../Testing/ from test.sh script and from regression test results
</PRE>

**3.19.1**
<PRE>
25/March/2020
in test.sh and testlen.sh - make TLSH_PROG a variable
</PRE>

**4.0.0**
<PRE>
26/March/2020
version 4: adding version identifier to each digest: 'T1'
	adding command line option -old to generate old style digests
	In this version - the showvers is defaulted to off - so this will pass the old regression tests
</PRE>

**4.0.1**
<PRE>
26/March/2020
version 4: adding version identifier to each digest: 'T1'
	turing on T1 functionality by setting showvers=1 in main
	updating regression tests to have T1 at the start of digests
</PRE>

**4.1.0**
<PRE>
26/March/2020
        adding -o option for output filename (output will go to stdout if no output file given)
		changed test scripts to use -o option
        adding -ojson option for json output
		added regression test for -ojson option
        adding -onull option to output empty files / files too small as TNULL
</PRE>

**4.2.0**
<PRE>
26/March/2020
	Windows version using minGW
</PRE>

**4.2.1**
<PRE>
27/March/2020
	resolve issue #78 json objects do not validate on windows
</PRE>

**4.2.2**
<PRE>
17/April/2020
	resolve issue #81
	Pass regression tests
</PRE>

**4.2.3**
<PRE>
22/April/2020
	add regression tests that are compatible with
		https://github.com/glaslos/tlsh
	To use
		$ cd Testing
		edit tlsh_go script to set prog= your Go TLSH application
		$ ./test.sh _go
</PRE>

**4.3.0**
<PRE>
26/June/2020
	issue #79 - divide by 0 if q3 == 0
		solution. if (q3 == 0) return invalid hash
</PRE>

**4.4.0**
<PRE>
08/Nov/2020
	Fixing Python Extension
	- updated python extension to T1 hashes (4.0.0)
	- fixed python_test.sh (which attempted to access old expected results files)
		now passes test
	- added license information to py_ext/tlshmodule.cpp
</PRE>

**4.4.1**
<PRE>
09/Dec/2020
	Command line options to tlsh_digest.py
		-conservative	enforce 256 byte limit
		-old		generate old style hash (without "T1")
	added python functions to tlsh package (for backwards compatibility)
		tlsh.oldhash(data)
		tlsh.conservativehash(data)
		tlsh.oldconservativehash(data)
</PRE>

**4.5.0**
<PRE>
10/Dec/2020
	Checking in files to create pypi package
</PRE>

**4.6.0**
<PRE>
23/04/2021
	Merging in pull requests
	issue #99 - new Java version that solves large file problem (Thanks Daniel)
	Add architecture ppc64le to travis build (Thanks ddeka2910)
	Fix tmpArray is undefined in JavaScript version (Thanks carbureted)
</PRE>

**4.7.0**
<PRE>
29/06/2021
	Release updated package py-tlsh on Pypi.org
	Merging in pull request that adds functions to Python package
		lvalue, q1ratio, q2ratio, checksum, bucket_value and is_valid
	resolve issue #102 - correct Python version numbers
</PRE>

**4.7.2**
<PRE>
02/07/2021
	Release updated package py-tlsh on Pypi.org
	regression tests for C++ and Python functions for:
		lvalue, q1ratio, q2ratio, checksum, bucket_value
	resolve issue #95 - allow Requires-Python: >=2.7
</PRE>

**4.8.0**
<PRE>
08/09/2021
	Merged in pull request 103
	Fix the make install target by adding the version.h in the installed files
</PRE>

**4.8.1**
<PRE>
09/09/2021
	Merged in pull request 107
	Improve portability, add shared library build, install tlsh_unittest
	Thanks to Dkapps for pull request 103
	Thanks to cgull  for pull request 107
</PRE>

**4.8.2**
<PRE>
09/09/2021
	4.8.2 release
	Merged in pull request 107
	Improve portability, add shared library build, install tlsh_unittest
	Thanks to Dkapps for pull request 103
	Thanks to cgull  for pull request 107
	fixed tlsh_win_version.h
</PRE>

**4.9.0**
<PRE>
10/09/2021
	use OPTION for command line options in make.sh
	consistent use of tabs for whitespace in make.sh
</PRE>

**4.9.1**
<PRE>
12/09/2021
	Added function to API
		int HistogramCount(int bucket);
	This function prints out the HistogramCount for a Bucket
	To see the HistogramCount, you need to call the constructor with fc_cons_option & 4 == 1
		(fc_cons_option is now a bitfield)
	test_parts.cpp shows you how to use this function.
	Added regression tests for HistogramCount(int bucket) into test_parts.sh
</PRE>

**4.9.2**
<PRE>
13/09/2021
	define various tlsh options using #define
		// #define     TLSH_OPTION_FORCE       1
		#define        TLSH_OPTION_CONSERVATIVE        2
		#define        TLSH_OPTION_KEEP_BUCKET         4
		#define        TLSH_OPTION_PRIVATE             8
		#define        TLSH_OPTION_THREADED            16
	use the #define in the code
</PRE>

**4.9.3**
<PRE>
13/09/2021
	added options -thread and -private
	-thread	the TLSH is evaluated with 2 threads (faster calculation)
		Only done for files / bytestreams >= 10000 bytes
		But this means that it is impossible to calculate the checksum
		So the checksum is set to zero
	-private
		Does not evaluate the checksum
		Useful if you do not want to leak information
		Slightly faster than default TLSH (code was written to optimize this)
Timing (using the utility provide "timing_unittest") : (On Mac 2.3 GHz)
Byte size: 1 million bytes

eval TLSH DEFAULT (4.9.3 compact hash 1 byte checksum sliding_window=5) 500 times...
TLSH(buffer) = T1A12500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
BEFORE	ms=1631512230350
AFTER	ms=1631512234041
TIME	ms=3691
TIME	ms=  7.38	per iteration

eval TLSH THREADED (4.9.3 compact hash 1 byte checksum sliding_window=5) 500 times...
TLSH(buffer) = T1002500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
BEFORE	ms=1631512234041
AFTER	ms=1631512236464
TIME	ms=2423
TIME	ms=  4.85	per iteration

eval TLSH PRIVATE (4.9.3 compact hash 1 byte checksum sliding_window=5) 500 times...
TLSH(buffer) = T1002500088C838B0A0F0EC3C0ACAB82F3B8228B0308CFA302338C0F0AE2C24F28000008
BEFORE	ms=1631512236464
AFTER	ms=1631512239485
TIME	ms=3021
TIME	ms=  6.04	per iteration

eval TLSH distance 50 million times...
Test 2: Calc distance TLSH digest
dist=138
BEFORE	ms=1631512239500
AFTER	ms=1631512240550
TIME	ms=1050
TIME	ms= 21.00	per million iterations
</PRE>

**4.10.0**
<PRE>
22/09/2021
	added Python tools for clustering file
		using DBSCAN
		using HAC-T
	we provide scripts to show people how to cluster the Malware Bazaar dataset using TLSH
</PRE>

**4.10.1**
<PRE>
30/09/2021
	merge in pull request 108 Configure CMake for pthreads
	fix to 48 Bucket hash
</PRE>

**4.11.0**
<PRE>
11/10/2021
	Jupyter notebook for Malware Bazaar analysis
	Clustered output and pattern file for Malware Bazaar
</PRE>

**4.11.1**
<PRE>
22/10/2021
	resolve issue #115
	CREATE_LINK does not work on CENTOS 7
</PRE>

**4.11.2**
<PRE>
23/10/2021
	resolve issue #116
	Library will not compile on CENTOS 7 (use of threads)
</PRE>
