[![Build Status](https://travis-ci.org/trendmicro/tlsh.svg?branch=master)](https://travis-ci.org/trendmicro/tlsh/)

# TLSH - Trend Micro Locality Sensitive Hash

TLSH is a fuzzy matching library.
Given a byte stream with a minimum length of 50 bytes
TLSH generates a hash value which can be used for similarity comparisons.
Similar objects will have similar hash values which allows for
the detection of similar objects by comparing their hash values.  Note that
the byte stream should have a sufficient amount of complexity.  For example,
a byte stream of identical bytes will not generate a hash value.

## What's New in TLSH 4.x.x
02/July/2021  
Release version 4.7.1
Updated Python realease with additional functions: 
* lvalue
* q1ratio
* q2ratio
* checksum
* bucket_value
* is_valid

23/April/2021  
Release version 4.6.0
Issue 99 raised issues about what to do when evaluating the TLSH for files over 4GB
We decided to define that TLSH is the TLSH of the first 4GB of a file

14/April/2021  
We have written technical material that focuses on 2 topics at [https://tlsh.org](https://tlsh.org/)
- fast nearest neighbour search and scalable clustering
- robustness to attack

2020
- adopted by [Virus Total](https://developers.virustotal.com/v3.0/reference#files-tlsh)
- adopted by [Malware Bazaar](https://bazaar.abuse.ch/api/#tlsh)

26/March/2020
- adding version identifier to the digest
- added output options (-o)
- added json object output (-ojson)
- added null digest (TNULL)

TLSH has gained some traction. It has been included in STIX 2.1 and been ported to a number of langauges.

We are adding a version identifier ("T1") to the start of the digest so that we can
cleary distinguish between different variants of the digest (such as non-standard choices of 3 byte checksum).
This means that we do not rely on the length of the hex string to determine if a hex string is a TLSH digest
(this is a brittle method for identifying TLSH digests).
We are doing this to enable compatibility, especially backwards compatibility of the TLSH approach.

This release will add "T1" to the start of TLSH digests.
The code is backwards compatible, it can still read and interpret 70 hex character strings as TLSH digests.
And data sets can include mixes of the old and new digests.
If you need old style TLSH digests to be outputted, then use the command line option '-old'

## Dedication

Thanks to Chun Cheng, who was a humble and talented engineer.

## Minimum byte stream length

The program in default mode requires an input byte stream with a minimum length of 50 bytes
(and a minimum amount of randomness - see note in Python extension below).

For consistency with older versions, there is a -conservative option which enforces a 256 byte limit.
See notes for version 3.17.0 of TLSH

## Computed hash

The computed hash is 35 bytes of data (output as 'T1' followed 70 hexidecimal characters. Total length 72 characters).
The 'T1' has been added as a version number for the hash - so that we can adapt the algorithm and still maintain
backwards compatibility.
To get the old style 70 hex hashes, use the -old command line option.

Bytes 3,4,5 are used to capture the information about the file as a whole
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

## Windows (MinGW)

Added in March 2020.
See the instructions in README.mingw

## Windows (Visual Studio)

Use the version-specific tlsh solution files ([tlsh.VC2005.sln](Windows/tlsh.VC2005.sln),
[tlsh.VC2008.sln](Windows/tlsh.VC2008.sln), ...) under the Windows directory.

See [tlsh.h](include/tlsh.h) for the tlsh library interface and [tlsh_unittest.cpp](test/tlsh_unittest.cpp) and
[simple_unittest.cpp](test/simple_unittest.cpp) under the `test` directory for example code.

## Using TLSH in Python

### Python Package

We have recently created a Python package on PyPi: [https://pypi.org/project/py-tlsh/](https://pypi.org/project/py-tlsh/)  
The py-tlsh replaces the python-tlsh package. For details see [issue 94](https://github.com/trendmicro/tlsh/issues/94)  
To install this package
```
	$  pip install py-tlsh
```

### Python Extension

If you need to build your own Python package, then there is a README.python with notes about the python version

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

#### Python Usage

```python
import tlsh

tlsh.hash(data)
```

Note data needs to be bytes - not a string.
This is because TLSH is for binary data and binary data can contain a NULL (zero) byte.

In default mode the data must contain at least 50 bytes to generate a hash value and that
it must have a certain amount of randomness.
To get the hash value of a file, try

```python
tlsh.hash(open(file, 'rb').read())
```

Note: the open statement has opened the file in binary mode.

#### Python Example
```python
import tlsh

h1 = tlsh.hash(data)
h2 = tlsh.hash(similar_data)
score = tlsh.diff(h1, h2)

h3 = tlsh.Tlsh()
with open('file', 'rb') as f:
    for buf in iter(lambda: f.read(512), b''):
        h3.update(buf)
    h3.final()
# this assertion is stating that the distance between a TLSH and itself must be zero
assert h3.diff(h3) == 0
score = h3.diff(h1)
```

#### Python Extra Options

The `diffxlen` function removes the file length component of the tlsh header from the comparison.

```python
tlsh.diffxlen(h1, h2)
```

If a file with a repeating pattern is compared to a file with only a single instance of the pattern,
then the difference will be increased if the file lenght is included.
But by using the `diffxlen` function, the file length will be removed from consideration.

#### Python Backwards Compatibility Options

If you use the "conservative" option, then the data must contain at least 256 characters.
For example,

```python
import os
tlsh.conservativehash(os.urandom(256))
```

should generate a hash, but

```python
tlsh.conservativehash(os.urandom(100))
```

will generate TNULL as it is less than 256 bytes.

If you need to generate old style hashes (without the "T1" prefix) then use

```python
tlsh.oldhash(os.urandom(100))
```


The old and conservative options may be combined:

```python
tlsh.oldconservativehash(os.urandom(500))
```

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
- For the 72 characters hash, there is a detailed table of experimental Detection rates and False Positive rates
  based on the threshhold. see [Table II on page 5](https://github.com/trendmicro/tlsh/blob/master/TLSH_CTC_final.pdf)

# Publications

- Jonathan Oliver, Chun Cheng, and Yanggui Chen, [TLSH - A Locality Sensitive Hash](https://github.com/trendmicro/tlsh/blob/master/TLSH_CTC_final.pdf).
4th Cybercrime and Trustworthy Computing Workshop, Sydney, November 2013
- Jonathan Oliver, Scott Forman, and Chun Cheng, [Using Randomization to Attack Similarity Digests](https://github.com/trendmicro/tlsh/blob/master/Attacking_LSH_and_Sim_Dig.pdf).
ATIS 2014, November, 2014, pages 199-210

# Current Version

**4.8.1**
<PRE>
09/09/2021
	Merged in pull request 107
	Improve portability, add shared library build, install tlsh_unittest
	Thanks to Dkapps for pull request 103
	Thanks to cgull  for pull request 107
</PRE>

# Change History

see [Change_History.md](Change_History.md)
