# TLSH - C++ extension for Python

[TLSH (Trend Micro Locality Sensitive Hash)](https://github.com/trendmicro/tlsh) is a fuzzy matching library.
Given a byte stream with a minimum length of 50 bytes
TLSH generates a hash value which can be used for similarity comparisons.
Similar objects will have similar hash values which allows for
the detection of similar objects by comparing their hash values.  Note that
the byte stream should have a sufficient amount of complexity.  For example,
a byte stream of identical bytes will not generate a hash value.

## What's new in py-tlsh 4.7.2
This Python module supercedes the python-tlsh package on PyPi.
The improvements in 4.7.2, are that we added additional functions to Python
* lvalue
* q1ratio
* q2ratio
* checksum
* bucket_value
* is_valid

The improvements 4.5.0 were:
* fixed this package so that it works on Windows
* compatibility with VirusTotal adoption of TLSH: updated to the T1 hash format with backwards compatibility for old hashes
* fixed the q3=0 divide by zero bug [issue 79](https://github.com/trendmicro/tlsh/issues/79)

## Usage

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

## Example
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

## Extra Options

The `diffxlen` function removes the file length component of the tlsh header from the comparison.

```python
tlsh.diffxlen(h1, h2)
```

If a file with a repeating pattern is compared to a file with only a single instance of the pattern,
then the difference will be increased if the file lenght is included.
But by using the `diffxlen` function, the file length will be removed from consideration.

## Backwards Compatibility Options

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
