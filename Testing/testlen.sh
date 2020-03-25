#!/bin/sh

TLSH_PROG=$1
HIGH_SEQ=$2
if test -z "$HIGH_SEQ"
then
	HIGH_SEQ=25
fi
	
rm -f testlen.txt

echo "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()" >  testlen.txt
echo "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()" >> testlen.txt
cat testlen.txt testlen.txt > testlen2.txt

for x in `seq 1 $HIGH_SEQ` ; do
	echo "iter $x"
	wc -c testlen2.txt
	../bin/$TLSH_PROG -f testlen2.txt
	#
	# grow the size of the file according to the Fibonacci sequence
	#
	cat testlen.txt testlen2.txt > testlen3.txt
	mv testlen2.txt testlen.txt
	mv testlen3.txt testlen2.txt
	echo
done

rm -f testlen.txt testlen2.txt testlen3.txt
