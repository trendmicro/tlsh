from os.path import join, realpath
import re

tlsh_256 = '-DBUCKETS_256'
tlsh_3b = '-DCHECKSUM_3B'

with open(join(realpath('..'), 'CMakeLists.txt'), 'r') as f:
  l = f.readline()
  while l:
    l = f.readline()
    m = re.search(r'set\s*\(TLSH_BUCKETS_128\s*1\)', l, re.I) 
    if m:
      tlsh_256 = ''
    m = re.search(r'set\s*\(TLSH_CHECKSUM_1B\s*1\)', l, re.I)
    if m:
      tlsh_3b = ''

import sys
import random
import tlsh

inserts = int(sys.argv[1])
checksums = {}

if tlsh_3b == '':
  buckets = 256.0
else:
  buckets = 16777216.0

# generate random strings, extract the checksum, compare all the checksums
for i in xrange(inserts):
  s = ''.join([`chr(j % 256)` for j in random.sample(xrange(10**9),512)])

  if tlsh_3b == '':
    checksum = int(tlsh.hash(s)[:2], 16)
  else:
    checksum = int(tlsh.hash(s)[:6], 16)

  if checksums.has_key(checksum):
    checksums[checksum] += 1
  else:
    checksums[checksum] = 1
  
print inserts - buckets * (1.0 - ((buckets - 1)/buckets)**inserts)

for k in checksums.keys():
  if checksums[k] > 1:
    print "collision at ", k, "for", checksums[k], "times"
