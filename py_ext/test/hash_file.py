import sys
import tlsh

with open(sys.argv[1], 'rb') as f:
  d = f.read()

h1 = tlsh.hash(d)
print h1
