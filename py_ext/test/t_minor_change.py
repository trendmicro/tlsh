import tlsh

diff = []

# 256 is the maximum number of single byte changes
for i in xrange(256):
  s1 = ''.join([`chr(j % 256)` for j in xrange(1000)])
  s2 = ''.join([`chr(j % 256)` for j in xrange(999)])

  k = (999 - i) % 256
  s2 = s2 + `chr(k)`
  h1 = tlsh.hash(s1)
  h2 = tlsh.hash(s2)
  diff.append(tlsh.diff(h1, h2))

for i in xrange(1,256):  
  print 'diff', i, 'score', diff[i]

# 0 has not change
print 'diff', 0, 'score', diff[0]
