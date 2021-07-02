import os
import re
import sys
from os.path import join
from codecs import open

from setuptools import setup, Extension

tlsh_256 = "-DBUCKETS_256"
tlsh_128 = "-DBUCKETS_128"
tlsh_3b = "-DCHECKSUM_3B"

here = os.path.abspath(os.path.dirname(__file__))

with open(os.path.join(here, "README.md"), encoding="utf-8") as f:
    readme = f.read()


def publish():
    os.system("twine upload dist/*")

if sys.argv[-1] == "publish":
    publish()
    sys.exit()

with open("CMakeLists.txt", "r") as f:
    l = f.readline()
    while l:
        l = f.readline()
        m = re.search(r"set\s*\(TLSH_BUCKETS_128\s*1\)", l, re.I)
        if m:
            tlsh_256 = ""
        m = re.search(r"set\s*\(TLSH_CHECKSUM_1B\s*1\)", l, re.I)
        if m:
            tlsh_3b = ""

if os.name == "nt":
    tlsh_module = Extension(
        "tlsh",
        sources=[
            "tlshmodule.cpp",
            join("src", "tlsh.cpp"),
            join("src", "tlsh_impl.cpp"),
            join("src", "tlsh_util.cpp"),
        ],
        include_dirs=[join("include"), join("Windows")],
        define_macros=[("WINDOWS", None)],
    )
else:
    tlsh_module = Extension(
        "tlsh",
        sources=[
            "tlshmodule.cpp",
            join("src", "tlsh.cpp"),
            join("src", "tlsh_impl.cpp"),
            join("src", "tlsh_util.cpp"),
        ],
        include_dirs=[join("include")],
    )

if tlsh_256 != "":
    tlsh_module.extra_compile_args.append(tlsh_256)
else:
    tlsh_module.extra_compile_args.append(tlsh_128)
if tlsh_3b != "":
    tlsh_module.extra_compile_args.append(tlsh_3b)

import setuptools


setuptools.setup(
    name="py-tlsh",
    version="4.7.2",
    description="TLSH (C++ Python extension)",
    long_description=readme,
    long_description_content_type="text/markdown",
    author="Jonathan Oliver / Chun Cheng / Yanggui Chen",
    author_email="jon_oliver@trendmicro.com",
    ext_modules=[tlsh_module],
    url="https://github.com/trendmicro/tlsh",
    license = "Apache or BSD",
    classifiers=[
        "Intended Audience :: Developers",
        "Development Status :: 5 - Production/Stable",
        "Natural Language :: English",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 2.7",
    ],
    python_requires='>=2.7',
)
