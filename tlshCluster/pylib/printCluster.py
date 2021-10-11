#################################################################
# TLSH is provided for use under two licenses: Apache OR BSD. Users may opt to use either license depending on the license
# restictions of the systems with which they plan to integrate the TLSH code.
#
# Apache License: # Copyright 2013 Trend Micro Incorporated
#
# Licensed under the Apache License, Version 2.0 (the "License");
# You may obtain a copy of the License at      http://www.apache.org/licenses/LICENSE-2.0
#
# BSD License: # Copyright (c) 2013, Trend Micro Incorporated. All rights reserved.
#
# see file "LICENSE
#################################################################

def printAllCluster(outfname, cenfname, cluster, memberList, tlshList, tobjList, labels, verbose):
	###########
	# cluster output file
	###########
	f = open(outfname, "w")
	if (f is None):
		print("error: cannot write  to ", outfname)
		return
	# end if
	###########
	# centers output file (csv file of cluster centers)
	###########
	cenf = None
	if (cenfname != ""):
		cenf = open(cenfname, "w")
		if (f is None):
			print("error: cannot write  to ", cenfname)
			return
		# end if
		cenf.write("tlsh,family,firstSeen,label,radius,nitems\n")
	# end if

	labelList = labels[0]
	dateList  = labels[1]

	for ci in range(0, len(memberList) ):
		ml = memberList[ci]
		if (len(ml) > 1):
			printCluster(f, cenf, ci, cluster, memberList, tlshList, tobjList, labelList, dateList)
		# end if
	# end for

	if (verbose >= 1):
		f.close()
		print("written ", outfname)
		if (cenfname != ""):
			cenf.close()
			print("written ", cenfname)
		# end if
	# end if

def estimateRadius(ml, tobjList):
	nlist = len(ml)

	#########################
	# sample max 100 points to calc radius
	#########################
	nsteps = 100
	jump = int(nlist / nsteps)
	maxni = jump * nsteps
	if (jump == 0):
		jump = 1
		maxni = nlist

	rad_cluster = 99999
	rad_idx     = -1

	for xi in range(0, maxni, jump):
		x = ml[xi]
		hx = tobjList[x]
		radx=0
		for yi in range(0, maxni, jump):
			y = ml[yi]
			if (x != y):
				hy = tobjList[y]
				d = hx.diff(hy)
				if (d > radx):
					radx = d
				# end if
			# end if
		# end for
		if (radx < rad_cluster):
			rad_cluster	= radx
			rad_idx		= x
		# end if
	# end for
	return(rad_cluster)

from collections import Counter

def printCluster(f, cenf, gA, cluster, memberList, tlshList, tobjList, labelList, dateList):
	outml = sorted(memberList[gA])
	rad_cluster = 99999
	rad_idx     = -1
	labelSet    = set()
	nitems = len(outml)
	for x in outml:
		hx = tobjList[x]
		radx=0
		for y in outml:
			if (x != y):
				hy = tobjList[y]
				d = hx.diff(hy)
				if (d > radx):
					radx = d
				# end if
			# end if
		# end for
		if (radx < rad_cluster):
			rad_cluster	= radx
			rad_idx		= x
		# end if
		if (labelList is not None) and (len(labelList) > 0):
			if (labelList[x] != "NO_SIG"):
				labelSet.add(labelList[x].lower())
	# end for
	################################
	# identify the most common label
	################################
	nlabel=len(labelSet)
	labelMostCommon = "NULL"
	if (nlabel == 0):
		labelStr = ""
	else:
		labelStr = str( sorted(list(labelSet)) )
		tmpList = [labelList[x] for x in outml if labelList[x] != "n/a"]
		if (len(tmpList) > 0):
			c = Counter(tmpList)
			labelMostCommonTuple	= c.most_common(1)[0]
			labelMostCommon		= labelMostCommonTuple[0] # end if # end if ################################ # identify the first time value
	################################
	firstSeen = "NULL"
	if (dateList is not None):
		clusterTimeList = [dateList[x] for x in outml]
		firstSeen = min( clusterTimeList )
		tmpList = [labelList[x] for x in outml if labelList[x] != "n/a"]
	# end if
	################################
	f.write("members:	" + str(outml) + "\n" )
	f.write("labels:	" + labelStr + "\n" )
	f.write("nlabels:	" + str(nlabel) + "\n" )
	f.write("nitems:	" + str(nitems) + "\n" )
	f.write("center:	" + tlshList[rad_idx] + "\n" )
	f.write("radius:	" + str(rad_cluster) + "\n" )
	if (len(labelList) > 0):
		for x in outml:
			f.write("	" + tlshList[x] + "	" + labelList[x] + "\n")
		# end for
	else:
		for x in outml:
			f.write("	" + tlshList[x] + "\n")
		# end for
	# end if
	if (cenf is not None):
		if ( labelMostCommon == "NULL" ):
			labelMostCommon = "Cluster " + str(gA)
		label_date = labelMostCommon + " " + firstSeen + " (" + str(nitems) + ")"

		cenf.write(tlshList[rad_idx]		+ "," )
		cenf.write(labelMostCommon		+ "," )
		cenf.write(firstSeen			+ "," )
		cenf.write(label_date			+ "," )
		cenf.write(str(rad_cluster)		+ "," )
		cenf.write(str(nitems)			)
		cenf.write("\n" )
	# end if
