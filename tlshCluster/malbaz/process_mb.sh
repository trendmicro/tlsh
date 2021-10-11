#!/bin/sh

if test -f full.csv
then
	echo "found full.csv"
else
	if test ! -f mb_download.zip
	then
		echo "You need to download the data (a zip file) from https://bazaar.abuse.ch/export/#csv "
		echo "	rename it as mb_download.zip"
		exit
	fi
	echo "unzip mb_download.zip"
	      unzip mb_download.zip
fi

if test -f mb_full.csv
then
	echo "found mb_full.csv"
else
	echo "cat full.csv | sed -f malware_bazaar.sed > mb_full.csv"
	      cat full.csv | sed -f malware_bazaar.sed > mb_full.csv
fi

regTest=1
if test $regTest = 1
then
	sampleSize="1000 2000 4000 8000"
else
	sampleSize="10000 20000 40000 80000"
fi

rm -f process_mb.log
for NLINES in $sampleSize ; do
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
	if test $regTest = 1
	then
		EDBSCAN=exp/dbscan_${NLINES}.out_EXP
		EHAC=exp/hac_${NLINES}.out_EXP
		if test ! -f dbscan_${NLINES}.out
		then
			echo "error: DBSCAN failed. Failed to create dbscan_${NLINES}.out"
			exit
		fi
		if test ! -f $EDBSCAN
		then
			echo "warning: no expected output file: $EDBSCAN"
		else
			diffc=`diff dbscan_${NLINES}.out $EDBSCAN | wc -l`
			if test $diffc = 0
			then
				echo "passed"
			else
				echo "error: unexpected output"
				echo "diff dbscan_${NLINES}.out $EDBSCAN"
				exit
			fi
		fi
		if test ! -f hac_${NLINES}.out
		then
			echo "error: HAC-T failed. Failed to create hac_${NLINES}.out"
			exit
		fi
		if test ! -f $EHAC
		then
			echo "warning: no expected output file: $EHAC"
		else
			diffc=`diff hac_${NLINES}.out $EHAC | wc -l`
			if test $diffc = 0
			then
				echo "passed"
			else
				echo "error: unexpected output"
				echo "diff hac_${NLINES}.out $EHAC"
				exit
			fi
		fi
	fi
done
echo "written process_mb.log"
grep ms process_mb.log
