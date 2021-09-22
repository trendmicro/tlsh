#!/bin/sh

REGRESSION_TEST_OPTIONS="-utest 1 -showtime 0"

for size in 448 10K ; do
	echo "========================"
	echo "    data set size: $size"
	echo "========================"

	if test $size = "448"
	then
		datafile="reg_test_448.csv"
		outfile="hac_out"
	else
		datafile="reg_test_10K.csv"
		outfile="hac10K_out"
	fi
	##############################
	# Test 1: strict / optimal clustering
	##############################
	echo "Test 1: strict / optimal clustering"
	echo "python ../hac-t.py $REGRESSION_TEST_OPTIONS -opt 1	-f $datafile -o $outfile.clusters > $outfile"
	      python ../hac-t.py $REGRESSION_TEST_OPTIONS -opt 1	-f $datafile -o $outfile.clusters > $outfile

	for file in $outfile $outfile.clusters ; do
		if test ! -f Exp_Results/$file.EXP
		then
			echo "failed to find Exp_Results/$file.EXP"
			exit
		fi
		if test ! -f $file
		then
			echo "failed to create $file"
			exit
		fi
		echo  "diff -w $file Exp_Results/$file.EXP | wc -l"
		ndiff=`diff -w $file Exp_Results/$file.EXP | wc -l`
		if test $ndiff -gt 0
		then
			echo "error: diff -w $file Exp_Results/$file.EXP"
			exit
		else
			echo "pass"
		fi
	done
	echo

	##############################
	# Test 2: default options
	##############################
	echo "Test 2: default options"
	echo "python ../hac-t.py		-f $datafile -o $outfile.default.clusters"
	      python ../hac-t.py		-f $datafile -o $outfile.default.clusters

	file="$outfile.default.clusters"
	if test ! -f Exp_Results/$file.EXP
	then
		echo "failed to find Exp_Results/$file.EXP"
		exit
	fi
	if test ! -f $file
	then
		echo "failed to create $file"
		exit
	fi
	echo  "diff -w $file Exp_Results/$file.EXP | wc -l"
	ndiff=`diff -w $file Exp_Results/$file.EXP | wc -l`
	if test $ndiff -gt 0
	then
		echo "error: diff -w $file Exp_Results/$file.EXP"
		exit
	else
		echo "pass"
	fi
	echo

	##############################
	# Test 3: allow stringy clusters
	##############################
	echo "Test 3: allow stringy clusters"
	echo "python ../hac-t.py -allow 1	-f $datafile -o $outfile.stringy.clusters"
	      python ../hac-t.py -allow 1	-f $datafile -o $outfile.stringy.clusters

	file="$outfile.stringy.clusters"
	if test ! -f Exp_Results/$file.EXP
	then
		echo "failed to find Exp_Results/$file.EXP"
		exit
	fi
	if test ! -f $file
	then
		echo "failed to create $file"
		exit
	fi
	echo  "diff -w $file Exp_Results/$file.EXP | wc -l"
	ndiff=`diff -w $file Exp_Results/$file.EXP | wc -l`
	if test $ndiff -gt 0
	then
		echo "error: diff -w $file Exp_Results/$file.EXP"
		exit
	else
		echo "pass"
	fi

	diffc=`diff $outfile.clusters $outfile.stringy.clusters | wc -l`
	if test $diffc -gt 0
	then
		echo "error: diff $outfile.clusters $outfile.stringy.clusters"
		exit
	fi
	echo

	##############################
	# Test 4: DBSCAN
	##############################
	echo "Test 4: DBSCAN"
	echo "python ../tlsh_dbscan.py -f $datafile -o $outfile.dbscan.clusters > $outfile.dbscan"
	      python ../tlsh_dbscan.py -f $datafile -o $outfile.dbscan.clusters > $outfile.dbscan

	file="$outfile.dbscan.clusters"
	if test ! -f Exp_Results/$file.EXP
	then
		echo "failed to find Exp_Results/$file.EXP"
		exit
	fi
	if test ! -f $file
	then
		echo "failed to create $file"
		exit
	fi
	echo  "diff -w $file Exp_Results/$file.EXP | wc -l"
	ndiff=`diff -w $file Exp_Results/$file.EXP | wc -l`
	if test $ndiff -gt 0
	then
		echo "error: diff -w $file Exp_Results/$file.EXP"
		exit
	else
		echo "pass"
	fi
	echo
done
