#!/bin/sh

if test -f full.csv
then
	echo "found full.csv"
else
	if test ! -f __QtrPfr.zip
	then
		echo "You need to download the data (a zip file) from https://bazaar.abuse.ch/export/#csv "
		exit
	fi
	echo "unzip __QtrPfr.zip"
	      unzip __QtrPfr.zip
fi

if test -f mb_full.csv
then
	echo "found mb_full.csv"
else
	echo "cat full.csv | sed -f malware_bazaar.sed > mb_full.csv"
	      cat full.csv | sed -f malware_bazaar.sed > mb_full.csv
fi

rm -f process_mb.log
### for NLINES in 10000 20000 40000 80000 ; do
for NLINES in 1000 2000 4000 8000 ; do
	if test -f mb_${NLINES}.csv
	then
		echo "found mb_${NLINES}.csv"
	else
		echo "head -n ${NLINES} mb_full.csv > mb_${NLINES}.csv"
		      head -n ${NLINES} mb_full.csv > mb_${NLINES}.csv
	fi

	echo "python ../tlsh_dbscan.py -showtime 1 -f mb_${NLINES}.csv -o dbscan_${NLINES}.out	>> process_mb.log"
	      python ../tlsh_dbscan.py -showtime 1 -f mb_${NLINES}.csv -o dbscan_${NLINES}.out	>> process_mb.log

	echo "python ../hac-t.py -showtime 1 -f mb_${NLINES}.csv -o hac_${NLINES}.out		>> process_mb.log"
	      python ../hac-t.py -showtime 1 -f mb_${NLINES}.csv -o hac_${NLINES}.out		>> process_mb.log
done
echo "written process_mb.log"
grep ms process_mb.log
