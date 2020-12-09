from __future__ import print_function

import sys
import tlsh

def compute_1(path, force_cons_param, old_param = 0):
    try:
        f = open(path, 'rb')
        data = f.read()
        if force_cons_param == 2:
            if old_param > 0:
                hs = tlsh.oldconservativehash(data)
            else:
                hs = tlsh.conservativehash(data)
        else:
            if old_param > 0:
            	hs = tlsh.oldhash(data)
            else:
            	hs = tlsh.hash(data)
        return hs
    except IOError as e:
        print('cannot find file: ', path)
    return ''

def main(argv):
    nargs=len(argv)
    if nargs < 2:
        print('usage: tlsh_digest.py [-force | -conservative] [-old] file')
        print('	-force:		Default behaviour. input string must be >= 50 char')
        print('	-conservative:	Only create a TLSH digest if the input string is >= 256 characters')
        print('	-old:		Backwards compatibility. Generate hash without T1 at the start of the hash')
        sys.exit()
    else:
        fname			= ''
        force_cons_param	= 0
        old_param		= 0

        for ai in range(1, nargs):
            if argv[ai] == '-force':
                force_cons_param=1
            elif argv[ai] == '-conservative':
                force_cons_param=2
            elif argv[ai] == '-old':
                old_param=1
            else:
                fname=argv[ai]
        # end for
        hex1 = compute_1(fname, force_cons_param, old_param)
        print(hex1, '\t', fname, sep='')
    # end if

main(sys.argv)
