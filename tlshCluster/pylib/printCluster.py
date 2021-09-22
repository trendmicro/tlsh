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

def printAllCluster(f, cluster, memberList, tlshList, tobjList, labelList):
	for ci in range(0, len(memberList) ):
		ml = memberList[ci]
		if (len(ml) > 1):
			printCluster(f, ci, cluster, memberList, tlshList, tobjList, labelList)
		# end if
	# end for

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

def printCluster(f, gA, cluster, memberList, tlshList, tobjList, labelList):
	outml = sorted(memberList[gA])
	rad_cluster = 99999
	rad_idx     = -1
	labelSet    = set()
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
		if (len(labelList) > 0):
			if (labelList[x] != "NO_SIG"):
				labelSet.add(labelList[x].lower())
	# end for
	nlabel=len(labelSet)
	if (nlabel == 0):
		labelStr = ""
	else:
		labelStr = str( sorted(list(labelSet)) )
	f.write("members:	" + str(outml) + "\n" )
	f.write("labels:	" + labelStr + "\n" )
	f.write("nlabels:	" + str(nlabel) + "\n" )
	f.write("nitems:	" + str(len(outml)) + "\n" )
	f.write("center:	" + str(rad_idx) + "\n" )
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
