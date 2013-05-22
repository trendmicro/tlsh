from distutils.core import setup, Extension
import os

tlsh_module = Extension('tlsh', \
  sources = ['tlshmodule.cpp', \
    os.path.join(os.path.realpath('..'), 'src', 'tlsh.cpp'), \
    os.path.join(os.path.realpath('..'), 'src', 'tlsh_impl.cpp'), \
    os.path.join(os.path.realpath('..'), 'src', 'tlsh_util.cpp') \
  ], \
  include_dirs = [os.path.join(os.path.realpath('..'), 'include')] \
)

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
