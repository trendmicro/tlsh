import os
import re
import sys
from os.path import join, realpath
from distutils.core import setup, Extension



tlsh_256 = "-DBUCKETS_256"
tlsh_128 = "-DBUCKETS_128"
tlsh_3b = "-DCHECKSUM_3B"

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

if os.name == 'nt':
  tlsh_module = Extension('tlsh', \
    sources = ['tlshmodule.cpp', \
      join(realpath('..'), 'src', 'tlsh.cpp'), \
      join(realpath('..'), 'src', 'tlsh_impl.cpp'), \
      join(realpath('..'), 'src', 'tlsh_util.cpp') \
    ], \
    include_dirs = [join(realpath('..'), 'include'),
                    join(realpath('..'), 'Windows')],\
    define_macros = [('WINDOWS', None),], \
  )
else:
  tlsh_module = Extension('tlsh', \
    sources = ['tlshmodule.cpp', \
      join(realpath('..'), 'src', 'tlsh.cpp'), \
      join(realpath('..'), 'src', 'tlsh_impl.cpp'), \
      join(realpath('..'), 'src', 'tlsh_util.cpp') \
    ], \
    include_dirs = [join(realpath('..'), 'include')],
  )

if tlsh_256 != '':
  tlsh_module.extra_compile_args.append(tlsh_256)
else:
  tlsh_module.extra_compile_args.append(tlsh_128)
if tlsh_3b != '':
  tlsh_module.extra_compile_args.append(tlsh_3b)

description = """A C++ extension for TLSH

Usage:
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
"""

setup (
    name = 'tlsh',
    version = '4.5.0',
    description="TLSH (C++ Python extension)",
    long_description = description,
    author="Jonathan Oliver / Chun Cheng / Yanggui Chen",
    author_email="jon_oliver@trendmicro.com",
    ext_modules = [tlsh_module],
    url="https://github.com/trendmicro/tlsh",
    license = "Apache or BSD",
    classifiers=[
        "Intended Audience :: Developers",
        "Development Status :: 5 - Production/Stable",
        "Natural Language :: English",
        "Operating System :: OS Independent",
    ]
)
