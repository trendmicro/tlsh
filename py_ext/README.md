# TLSH - C++ extension for Python

[TLSH (Trend Micro Locality Sensitive Hash)](https://github.com/trendmicro/tlsh) is a fuzzy matching library.
Given a byte stream with a minimum length of 50 bytes
TLSH generates a hash value which can be used for similarity comparisons.
Similar objects will have similar hash values which allows for
the detection of similar objects by comparing their hash values.  Note that
the byte stream should have a sufficient amount of complexity.  For example,
a byte stream of identical bytes will not generate a hash value.

## Usage

```python
import tlsh

tlsh.hash(data)
```


Note that in default mode the data must contain at least 50 bytes to generate a hash value and that
it must have a certain amount of randomness.
If you use the "conservative" option, then the data must contain at least 256 characters.
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
assert h3.diff(h) == 0
score = h3.diff(h1)
```

