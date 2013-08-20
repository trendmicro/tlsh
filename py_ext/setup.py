from distutils.core import setup, Extension
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

tlsh_module = Extension('tlsh', \
  sources = ['tlshmodule.cpp', \
    join(realpath('..'), 'src', 'tlsh.cpp'), \
    join(realpath('..'), 'src', 'tlsh_impl.cpp'), \
    join(realpath('..'), 'src', 'tlsh_util.cpp') \
  ], \
  include_dirs = [join(realpath('..'), 'include')], \
)

if tlsh_256 != '':
  tlsh_module.extra_compile_args.append(tlsh_256)
if tlsh_3b != '':
  tlsh_module.extra_compile_args.append(tlsh_3b)

description = """A C++ extension for TLSH

Usage:
import tlsh
h = tlsh.hash(data)
score = tlsh.diffscore(h1, h2)
"""

setup (name = 'tlsh',
  version = '0.1.0',
  description = 'TLSH (C++ version)',
    long_description = description,
    author = "Chun Cheng",
    ext_modules = [tlsh_module])
