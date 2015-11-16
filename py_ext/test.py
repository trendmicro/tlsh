from __future__ import print_function

import sys
import tlsh

def compute_1(path):
    with open(path, 'rb') as f:
        data = f.read()
        hs = tlsh.hash(data)
    return hs

def compute_2(path):
    h = tlsh.Tlsh()
    with open(path, 'rb') as f:
        for buf in iter(lambda: f.read(512), b''):
            h.update(buf)
    h.final()
    return h

hex1 = compute_1(sys.argv[1])
print('tlsh.hash hex1', hex1)
hex2 = compute_1(sys.argv[2])
print('tlsh.hash hex2', hex2)
print('tlsh.diff(hex1, hex2)', tlsh.diff(hex1, hex2))
print('tlsh.diff(hex2, hex1)', tlsh.diff(hex2, hex1))

h1 = compute_2(sys.argv[1])
hex1 = h1.hexdigest()
print('tlsh.Tlsh hex1', hex1)
h2 = compute_2(sys.argv[2])
hex2 = h2.hexdigest()
print('tlsh.Tlsh hex2', hex2)
print('h1.diff(h2)', h1.diff(h2))
print('h2.diff(h1)', h2.diff(h1))
print('h1.diff(hex2)', h1.diff(hex2))
print('h2.diff(hex1)', h2.diff(hex1))

h3 = tlsh.Tlsh()
h3.fromTlshStr(hex2)
print('tlsh.Tlsh.fromTlshStr', hex2)
print('h3.diff(h2)', h3.diff(h2))


