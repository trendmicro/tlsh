from __future__ import print_function

import sys
import tlsh

def compute(path):
    h = tlsh.Tlsh()
    with open(path, 'rb') as f:
        for buf in iter(lambda: f.read(512), b''):
            h.update(buf)
    h.final()
    return h

h1 = compute(sys.argv[1])
hex1 = h1.hexdigest()
print('hex1', hex1)
h2 = compute(sys.argv[2])
hex2 = h2.hexdigest()
print('hex2', hex2)
print('tlsh.diff(hex1, hex2)', tlsh.diff(hex1, hex2))
print('tlsh.diff(hex2, hex1)', tlsh.diff(hex2, hex1))
print('h1.diff(h2)', h1.diff(h2))
print('h2.diff(h1)', h2.diff(h1))
print('h1.diff(hex2)', h1.diff(hex2))
print('h2.diff(hex1)', h2.diff(hex1))
