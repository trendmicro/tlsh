from __future__ import print_function

import sys
import tlsh

def compute_1(path,force):
    try:
        f = open(path, 'rb')
        data = f.read()
        if force == 1:
            hs = tlsh.forcehash(data)
        else:
            hs = tlsh.hash(data)
        return hs
    except IOError as e:
        print('cannot find file: ', path)
    return ''

def main(argv):
    nargs=len(argv)
    if nargs < 2:
        print('usage: tlsh_digest.py [-force] file')
        sys.exit()
    else:
        if argv[1] == '-force':
            force=1
            fname=argv[2]
        else:
            force=0
            fname=argv[1]
        hex1 = compute_1(fname, force)
        print(hex1, '\t', fname, sep='')

main(sys.argv)
