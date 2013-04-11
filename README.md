TLSH
=======================================

TLSH is a fuzzy matching library. Given a binary object, it generates a hash 
value. The hash values can be used for similarity comparison. Similary 
objects have similar hash values. Similar hash values signal similar objects.

The output hash is 35 bytes long. The first 3 bytes are used to capture the 
global similarity. The last 32 bytes are used to capture the local similarity.

Here are the design choices.
* To improve comparison accuracy, TLSH tracks counting bucket height 
distribution in quartiles. Bigger quartile difference results in higher 
difference score.
* Use specially 6 trigrams to give equal representation of the bytes in the 5 
byte sliding window which produces improved results.
* Pearson hash is used to distribute the trigram counts to the counting buckets.
* The global similarity score distances objects with significant size 
difference. It also distances objects with different quartile distributions.

TLSH similarity score of 256 means the objects are almost identical. A score of 
128 means the objects are very different. 

Get Started
=======================================

TLSH is public project hosted on github.com. Clone the project from 
git@github.com:trendmicro/tlsh.git. Run CMake to create the Makefile and
then make the project. A static library will be created under the "lib" directory.
Look under the "test" directory for example code. 
<pre>
  git clone git@github.com:trendmicro/tlsh.git
  cd tlsh
  mkdir -p build/release
  cd build/release
  cmake ../..
  make
  make test
</pre>  
Changes
=======================================
1.0.0:
- Implement TLSH.

2.0.0:
- Enhance TLSH to allow construct TLSH from hash string.

3.0.0:
- Bug fixes.
- Update to build with CMake.
- Publish on github.

Committers
=======================================
tlsh@trendmicro.com

