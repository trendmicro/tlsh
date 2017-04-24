#!/bin/sh

file=$1
if test -z "$file"
then
	echo "demo.sh PEFILE"
	exit
fi
if test ! -f $file
then
	echo "error: cannot open $file"
	exit
fi

rm -f matches.csv

python tlsh_bh_tool.py $file 2> .tmp.err

echo
echo "Results of calling:"
echo "\$ python tlsh_bh_tool.py $file"
echo

if test ! -f matches.csv
then
	echo "no result"
	exit
fi

sha256=`grep -v "sha256,tlsh,id,tag,distance_score" matches.csv | cut -f 1 -d, | head -1`
tlsh=`  grep -v "sha256,tlsh,id,tag,distance_score" matches.csv | cut -f 2 -d, | head -1`
found=` grep -v "sha256,tlsh,id,tag,distance_score" matches.csv | cut -f 3 -d, | head -1`
dist=`  grep -v "sha256,tlsh,id,tag,distance_score" matches.csv | cut -f 5 -d, | head -1`

echo
echo "FILE	$file"
echo "SHA256	$sha256"
echo "TLSH	$tlsh"
echo "Similar	$found"
echo "Distance	$dist"
echo
echo Details
echo =======
echo

cut -f 3- -d, matches.csv

echo
list=` grep -v "sha256,tlsh,id,tag,distance_score" matches.csv | cut -f 3 -d, | head -5`
for x in $list ; do
	# echo "curl -s \"https://grid-global.trendmicro.com/rs/level-0/files/isKnownGood;api-key=e4c9a1a0-2833-11e4-8c21-0800200c9a66/$x\""
	res=` curl -s  "https://grid-global.trendmicro.com/rs/level-0/files/isKnownGood;api-key=e4c9a1a0-2833-11e4-8c21-0800200c9a66/$x"`
	if test "$res" = "true"
	then
		echo "$x on Whitelist"
		exit
	fi
done
