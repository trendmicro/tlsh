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

# experiments (for timing etc)
# sampleSize="10000 20000 40000 80000 120000 160000"
sampleSize="389300"

# rm -f pattern_mb.log
for NLINES in $sampleSize ; do
	if test -f mb_${NLINES}.csv
	then
		echo "found mb_${NLINES}.csv"
	else
		echo "head -n ${NLINES} mb_full.csv > mb_${NLINES}.csv"
		      head -n ${NLINES} mb_full.csv > mb_${NLINES}.csv
	fi
	if test -f clust_${NLINES}.csv
	then
		echo "found clust_${NLINES}.csv"
	else
		echo "python ../hac-t.py -showtime 1 -f mb_${NLINES}.csv -o hac_${NLINES}.out -oc clust_${NLINES}.csv	>> pattern_mb.log"
		      python ../hac-t.py -showtime 1 -f mb_${NLINES}.csv -o hac_${NLINES}.out -oc clust_${NLINES}.csv	>> pattern_mb.log
		if test ! -f clust_${NLINES}.csv
		then
			echo "error: failed to write clust_${NLINES}.csv"
			exit
		fi
		echo "written clust_${NLINES}.csv"
	fi
done
#############################
# grep for timing experiments
# grep ms pattern_mb.log
#############################

CFILE=clust_${NLINES}.csv
###################################
# now we convert the cluster output file into a .pat file so that tlsh_pattern can process it
###################################
# tlsh,family,firstSeen,label,radius,nitems
#	=>
# col 1: pattern number
# col 2: nitems in group
# col 3: TLSH
# col 4: radius
# col 5: pattern label

cut -f 6 -d, $CFILE | tail +2 > tmp_pat.nitems
cut -f 1 -d, $CFILE | tail +2 > tmp_pat.tlsh
cut -f 5 -d, $CFILE | tail +2 > tmp_pat.radius
cut -f 2 -d, $CFILE | tail +2 > tmp_pat.pat_label
nlines=`cat tmp_pat.tlsh | wc -l`
seq $nlines	    > tmp_pat.lineno

###########################
# see the malbaz.ipynb
#	Mirai / Gafgyt
#	in that notebook we see that Mirai and Gafgyt share code and get mis-labelled for each other
#	https://securityaffairs.co/wordpress/116882/cyber-crime/gafgyt-re-uses-mirai-code.html
#	We therefore put them in the MiraiGafgyt group
#
#	AgentTesla / SnakeKeylogger / Formbook / Loki
#	The notebook highlights that these malware families are difficult to distinguish
#	https://cyberintelmag.com/malware-viruses/year-long-spear-phishing-campaign-targets-energy-sector-with-agent-tesla-other-rats/
#	and get mis-lablled as each other
#	We therefore put them in the TeslaGroup
###########################
cat tmp_pat.pat_label | sed -f pattern_mb.sed > tmp_pat.pat_label.groups

paste tmp_pat.lineno tmp_pat.nitems tmp_pat.tlsh tmp_pat.radius tmp_pat.pat_label.groups > clust_${NLINES}.pat
echo "written clust_${NLINES}.pat"

rm -f tmp_pat.lineno	tmp_pat.nitems	tmp_pat.pat_label	tmp_pat.pat_label.groups	tmp_pat.radius	tmp_pat.tlsh
