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

import sys
import tlsh

from pylib.myheap import *
from pylib.printCluster import *
from pylib.tlsh_lib import *

###################################
# Global Vars
###################################

linearCheck = False
metricCheck = False
hac_allowStringyClusters = False
hac_verbose    = 0

def median(currlist):
	newlist = sorted(currlist)
	listlen = len(currlist)
	mid = int((listlen-1)/2)
	return newlist[mid]

###################################
# VP Tree
###################################

class Node:
	def __init__(self, point, tobj=None, idx=-1, threshold=0):
		self.LC		= None
		self.RC		= None
		self.point	= point
		self.tobj	= tobj
		self.idx	= idx
		self.threshold	= threshold

	def insert(self, point):
# Compare the new value with the parent node
		if self.point:
			if point < self.point:
				if self.LC is None:
					self.LC = Node(point)
				else:
					self.LC.insert(point)
			elif point > self.point:
				if self.RC is None:
					self.RC = Node(point)
				else:
					self.RC.insert(point)
		else:
			self.point = point

# Print the tree
	def PrintTree(self,maxdepth,depth):
		if (depth > maxdepth):
			print( "..." )
			return
		# end if
		if self.LC:
			self.LC.PrintTree(maxdepth,depth+1)
		print ( depth * "	", end="" )
		if (self.threshold == -1):
			print( "LEAF:  idx=" + str(self.idx) + " " + self.point )
		else:
			print( "SPLIT: idx=" + str(self.idx) + " " + self.point + " T=" + str(self.threshold)),
		# end if
		if self.RC:
			self.RC.PrintTree(maxdepth,depth+1)

hac_nDistCalc=0

def hac_resetDistCalc():
	global hac_nDistCalc
	hac_nDistCalc=0

def hac_lookupDistCalc():
	global hac_nDistCalc
	return(hac_nDistCalc)

def VPTGrow(tlshList, tobjList, tidxList):
	lenList = len(tlshList)
	if (lenList == 0):
		return(None)

	vpTlsh = tlshList[0]
	vpObj  = tobjList[0]
	vpIdx  = tidxList[0]

	if (lenList == 1):
		thisNode = Node(vpTlsh, vpObj, vpIdx, -1)
		return(thisNode)
	# end if

	global hac_nDistCalc
	hac_nDistCalc += len(tobjList)
	distList = [vpObj.diff(h1)  for h1 in tobjList]
	med = median(distList)
	# if (med == 0):
	# 	print("med = 0")
	# 	print(distList)
	thisNode = Node(vpTlsh, vpObj, vpIdx, med)

	tlshLeft  = []
	tobjLeft  = []
	tidxLeft  = []
	tlshRight = []
	tobjRight = []
	tidxRight = []
	for li in range(1, lenList):
		if (distList[li] < med):
			tlshLeft.append(tlshList[li])
			tobjLeft.append(tobjList[li])
			tidxLeft.append(tidxList[li])
		else:
			tlshRight.append(tlshList[li])
			tobjRight.append(tobjList[li])
			tidxRight.append(tidxList[li])
		# end if
	# end for
	thisNode.LC  = VPTGrow(tlshLeft,  tobjLeft,  tidxLeft)
	thisNode.RC  = VPTGrow(tlshRight, tobjRight, tidxRight)
	return(thisNode)

def distMetric(tobj, searchItem):
	global hac_nDistCalc
	hac_nDistCalc += 1
	d = searchItem.diff(tobj)
	return(d)

extra_constant = 20

def VPTSearch(node, searchItem, searchIdx, cluster, notInC, best):
	if node is None :
		return
	# end if
	d = distMetric(node.tobj, searchItem)
	if ((cluster[node.idx] != notInC) and (d < best['dist'])):
		best['dist']	= d
		best['point']	= node.point
		best['idx']	= node.idx
	# end if
	if d <= node.threshold:
		VPTSearch(node.LC, searchItem, searchIdx, cluster, notInC, best)
		if (d + best['dist'] + extra_constant >= node.threshold):
			VPTSearch(node.RC, searchItem, searchIdx, cluster, notInC, best)
		else:
			if (metricCheck):
				rightbest = { "dist":best['dist'], "point":None, "idx":best['idx'] }
				VPTSearch(node.RC, searchItem, searchIdx, cluster, notInC, rightbest)
				if (rightbest['idx'] != best['idx']):
					print("found problem right")
					print("best:", best)
					print("d:", d)
					print("threshold:", node.threshold)
					print(rightbest)
					sys.exit(1);
			# end if
		# end if
	else:
		VPTSearch(node.RC, searchItem, searchIdx, cluster, notInC, best)
		if (d - best['dist'] - extra_constant <= node.threshold):
			VPTSearch(node.LC, searchItem, searchIdx, cluster, notInC, best)
		else:
			if (metricCheck):
				leftbest = { "dist":best['dist'], "point":None, "idx":best['idx'] }
				VPTSearch(node.LC, searchItem, searchIdx, cluster, notInC, leftbest)
				if (leftbest['idx'] != best['idx']):
					print("found problem left")
					print("best:", best)
					print("d:", d)
					print("threshold:", node.threshold)
					print(leftbest)
					sys.exit(1);
			# end if
		# end if
	# end if

def Tentative_Merge(gA, gB, cluster, memberList, tlshList, tobjList, rootVPT, CDist):
	global hac_verbose
	membersA = memberList[gA]
	for A in membersA:
		best = { "dist":99999, "point":None, "idx":-1 }
		searchItem = tobjList[A]
		VPTSearch(rootVPT, searchItem, A, cluster, gA, best)
		dist = best['dist']
		B = best['idx']
		if (dist <= CDist) and (cluster[A] != cluster[B]):
			### print("sucess Tentative_Merge gA=", gA, " gB=", gB)
			### print("A:", tlshList[A] )
			### print("B:", tlshList[B] )
			### print("dist:", dist )
			### print("A=", A, best);

			### print("before merge", gA, gB)
			### printCluster(sys.stdout, gA, cluster, memberList, tlshList, tobjList, None)
			### printCluster(sys.stdout, gB, cluster, memberList, tlshList, tobjList, None)

			if (hac_verbose >= 1):
				print("Merge(2) A=", A, " B=", B, " dist=", dist)
			newCluster = Merge(cluster[A], cluster[B], cluster, memberList, tobjList, dist)
			if (hac_verbose >= 2):
				print("sucess Tentative_Merge gA=", gA, " gB=", gB)
			return(1)
		# end if
	# end for
	if (hac_verbose >= 2):
		print("failed Tentative_Merge gA=", gA, " gB=", gB)
	return(0)

def Merge(gA, gB, cluster, memberList, tobjList, dist):
	# radA = estimateRadius(memberList[gA], tobjList)
	# radB = estimateRadius(memberList[gB], tobjList)
	# print("before merge", gA, gB)
	# printCluster(gA, cluster, memberList)
	# printCluster(gB, cluster, memberList)
	if (gA == gB):
		print("warning in Merge gA=", gA, " gB=", gB)
		return(gA)
	# end if

	minA = min(memberList[gA])
	minB = min(memberList[gB])
	#################
	# the new cluster is the one with the smallest element
	#################
	if (minA < minB):
		c1 = gA
		c2 = gB
	else:
		c1 = gB
		c2 = gA
	# end if

	membersA = memberList[c1]
	for x in memberList[c2]:
		### print("x=", x)
		membersA.append(x)
		cluster[x] = c1
	memberList[c2] = []
	
	# print("after merge", gA, gB)
	# printCluster(gA, cluster, memberList)
	# printCluster(gB, cluster, memberList)
	# radc1 = estimateRadius(memberList[c1], tobjList)
	# if (radc1 > 30):
	#	print("ERROR before merge:	rad(A)=", radA, " rad(B)=", radB, " dist=", dist, "	after rad=", radc1)
	return(c1)

def linearSearch(searchItem, tobjList, ignoreList, linbest):
	bestScore = 9999999
	bestIdx   = -1
	for ti in range(0, len(tobjList)):
		if (ti not in ignoreList):
			h1 = tobjList[ti]
			d = searchItem.diff(h1)
			if (d < bestScore):
				bestScore = d
				bestIdx   = ti
			# end if
		# end if
	# end for
	linbest['dist'] = bestScore
	linbest['idx']  = bestIdx

def VPTsearch_add_to_heap(A, cluster, tobjList, rootVPT, heap):
	best = { "dist":99999, "point":None, "idx":-1 }
	searchItem = tobjList[A]
	ignoreList = [A]
	VPTSearch(rootVPT, searchItem, A, cluster, cluster[A], best)
	dist = best['dist']
	if (dist < 99999):
		B = best['idx']
		rec = { 'pointA': A, 'pointB': B, 'dist':dist }
		heap.insert(rec, dist)
		### :print("heap insert: ", rec)

		if (linearCheck):
			linbest = { "dist":99999, "point":None, "idx":-1 }
			linearSearch(searchItem, tobjList, ignoreList, linbest)
			lindist = linbest['dist']
			linB    = linbest['idx']
			if (lindist < dist):
				print("error: dist=",	 dist,	  "B=", B)
				print("error: lindist=", lindist, "linB=", linB)
				sys.exit()
			# end if
		# end if
	# end if

import datetime
import time

showTiming	= 1
prev		= None
startTime	= None

showNumberClusters	= 0

def setNoTiming():
	global showTiming
	showTiming	= 0

def setShowNumberClusters():
	global showNumberClusters
	showNumberClusters	= 1

def print_time(title, final=0):
	global showTiming
	global prev
	global startTime
	if (showTiming == 0):
		return

	now = datetime.datetime.now()
	print(title + ":	" + str(now))
	if (prev is None):
		startTime	= now
	else:
		tdelta = (now - prev)
		delta_micro	= tdelta.microseconds + tdelta.seconds * 1000000
		delta_ms	= int( delta_micro / 1000 )
		print(title + "-ms:	"  + str(delta_ms))
	# end if
	if (final == 1):
		tdelta = (now - startTime)
		delta_micro	= tdelta.microseconds + tdelta.seconds * 1000000
		delta_ms	= int( delta_micro / 1000 )
		print("time-ms:	"  + str(delta_ms))
	# end if
	prev = now

def print_number_clusters(memberList, end=False):
	count = 0
	single = 0
	for ci in range(0, len(memberList)):
		ml = memberList[ci]
		if (len(ml) == 1):
			single += 1
		elif (len(ml) > 1):
			count += 1
		# end if
	# end for
	if (end):
		print("ENDncl=", count, "	nsingle=", single)
	else:
		print("ncl=", count, "	nsingle=", single)

def HAC_T_step3(tlshList, tobjList, CDist, rootVPT, memberList, cluster):
	global hac_verbose

	ITERATION = 1
	clusters_to_examine = []
	for ci in range(0, len(memberList)):
		ml = memberList[ci]
		if (len(ml) > 1):
			clusters_to_examine.append(ci)
		# end if
	# end for

	while(len(clusters_to_examine) > 0):
		global showNumberClusters
		if (hac_verbose >= 1) or (showNumberClusters >= 1):
			print("ITERATION ", ITERATION)
			print_number_clusters(memberList)
		# end if

		lmodified = []
		for ci in clusters_to_examine:
			ml = memberList[ci]
			if (hac_verbose >= 2):
				print("checking cluster: ci=", ci, " ", ml)
			for A in ml:
				best = { "dist":99999, "point":None, "idx":-1 }
				searchItem = tobjList[A]
				VPTSearch(rootVPT, searchItem, A, cluster, cluster[A], best)
				dist = best['dist']
				if dist <= CDist:
					B = best['idx']

					mergeOK = True
					if (not hac_allowStringyClusters):
						newml = memberList[cluster[A]] + memberList[cluster[B]]
						newrad = estimateRadius(newml, tobjList)
						if (newrad > CDist):
							if (hac_verbose >= 2):
								radA = estimateRadius(memberList[cluster[A]], tobjList)
								radB = estimateRadius(memberList[cluster[B]], tobjList)
								print("failed merge:	dist(A=", A, ",B=", B, ") =", dist, " rad(A)=", radA, " rad(B)=", radB, " newrad=", newrad)
							mergeOK = False
						# end if
					# end if
					if (mergeOK):
						if (hac_verbose >= 2):
							print("merging as dist(A=", A, ",B=", B, ") =", dist, " need to go again...")
							printCluster(sys.stdout, cluster[A], cluster, memberList, tlshList, tobjList, None)
							printCluster(sys.stdout, cluster[B], cluster, memberList, tlshList, tobjList, None)

						if (hac_verbose >= 1):
							print("Merge(3) A=", A, " B=", B, " dist=", dist)
						newCluster = Merge(cluster[A], cluster[B], cluster, memberList, tobjList, dist)
						if (newCluster not in lmodified):
							lmodified.append(newCluster)
						break;
					# end if
				# end if
			# end for
		# end for
		clusters_to_examine = lmodified
		ITERATION += 1
	# end for
	return(ITERATION)

def HAC_T_opt(fname, CDist, step3, outfname, cenfname, verbose=0):
	global hac_verbose
	hac_verbose = verbose
	global hac_allowStringyClusters
	hac_allowStringyClusters = True

	##########################
	# Step 0: read in data / grow VPT
	##########################
	(tlshList, tobjList, labels) = read_data(fname)
	tidxList = range(0, len(tlshList) )

	##########################
	# Step 1: Preprocess data / Grow VPT
	##########################
	ndata	= len(tlshList)
	if (ndata >= 1000):
		print_time("Start")

	Dn	= range(0,ndata)
	rootVPT = VPTGrow(tlshList, tobjList, tidxList)

	cluster    = list(range(0, ndata))
	heap=MinHeap()
	for A in Dn:
		VPTsearch_add_to_heap(A, cluster, tobjList, rootVPT, heap)
	# end for

	if (ndata >= 1000):
		print_time("End-Step-1")

	##########################
	# Step 2: Cluster data (HAC_T_opt)
	##########################

	memberList = []
	for A in Dn:
		mlist = [ A ]
		memberList.append(mlist)
	# end for
	global showNumberClusters
	if (hac_verbose >= 1) or (showNumberClusters >= 1):
		print_number_clusters(memberList)

	while heap.nelem() > 0:
		rec = heap.deleteTop()
		A = rec['pointA']
		B = rec['pointB']
		d = rec['dist']
		if (d <= CDist) and (cluster[A] != cluster[B]):
			newCluster = Merge(cluster[A], cluster[B], cluster, memberList, tobjList, d)
		# end if
	# end while
	if (hac_verbose >= 1) and (ndata >= 1000):
		print_time("End-Step-2")

	##########################
	# Step 3: Find clusters which need to be merged
	##########################
	if (hac_verbose >= 1) or (showNumberClusters >= 1):
		print_number_clusters(memberList)
	if (step3 == 0):
		if (hac_verbose >= 1):
			print_time("Not-doing-Step-3", 1)
	else:
		HAC_T_step3(tlshList, tobjList, CDist, rootVPT, memberList, cluster)
		if (hac_verbose >= 1) and (ndata >= 1000):
			print_time("End-Step-3", 1)
	# end if
	if (hac_verbose >= 1) or (showNumberClusters >= 1):
		print_number_clusters(memberList, True)
	printAllCluster(outfname, cenfname, cluster, memberList, tlshList, tobjList, labels, hac_verbose)

def HAC_T(fname, CDist, step3, outfname, cenfname, allowStringy=0, verbose=0):
	global hac_verbose
	hac_verbose = 0
	global hac_allowStringyClusters
	hac_allowStringyClusters = allowStringy

	##########################
	# Step 0: read in data / grow VPT
	##########################
	(tlshList, tobjList, labels) = read_data(fname)
	tidxList = range(0, len(tlshList) )

	##########################
	# Step 1: Initialise / Grow VPT
	##########################
	ndata	= len(tlshList)
	if (hac_verbose >= 1) and (ndata >= 1000):
		print_time("Start")

	Dn	= range(0,ndata)
	rootVPT = VPTGrow(tlshList, tobjList, tidxList)

	##########################
	# Step 2: Cluster data
	##########################
	cluster    = list(range(0, ndata))
	memberList = []
	for A in Dn:
		mlist = [ A ]
		memberList.append(mlist)
	# end for
	if (hac_verbose >= 1) or (showNumberClusters >= 1):
		print_number_clusters(memberList)

	tent_heap=MinHeap()
	tent_dict = {}
	for A in Dn:
		best = { "dist":99999, "point":None, "idx":-1 }
		searchItem = tobjList[A]
		VPTSearch(rootVPT, searchItem, A, cluster, cluster[A], best)
		dist = best['dist']
		B = best['idx']
		if (hac_verbose >= 2):
			print("VPT: A=", A, " B=", B, " dist=", dist)
		if (B != -1) and (cluster[A] == cluster[B]):
			print("error: A=", A, "B=", B)
			sys.exit(1)
		# end if
		if (dist <= CDist):
			mergeOK = True
			if (not hac_allowStringyClusters):
				newml = memberList[cluster[A]] + memberList[cluster[B]]
				newrad = estimateRadius(newml, tobjList)
				if (newrad > CDist):
					if (hac_verbose >= 2):
						radA = estimateRadius(memberList[cluster[A]], tobjList)
						radB = estimateRadius(memberList[cluster[B]], tobjList)
						print("failed merge:	dist(A=", A, ",B=", B, ") =", dist, " rad(A)=", radA, " rad(B)=", radB, " newrad=", newrad)
					mergeOK = False
				# end if
			# end if
			if (mergeOK):
				if (hac_verbose >= 1):
					print("Merge(1) A=", A, " B=", B, " dist=", dist)
				newCluster = Merge(cluster[A], cluster[B], cluster, memberList, tobjList, dist)
			# end if
		elif (dist <= 2 * CDist) and (hac_allowStringyClusters):
			if (hac_verbose >= 2):
				print("Tentative_Merge A=", A, " B=", B, " dist=", dist)
			cluster1 = cluster[A]
			cluster2 = cluster[B]
			if (cluster1 < cluster2):
				tent2 = str(cluster1) + ":" + str(cluster2)
			else:
				tent2 = str(cluster2) + ":" + str(cluster1)
			# end if
			if (tent2 not in tent_dict):
				tent_dict[tent2] = 1
				rec = { 'pointA': A, 'pointB': B, 'dist':dist }
				tent_heap.insert(rec, dist)
			# end if
		# end if
	# end for
	if (hac_verbose >= 1) or (showNumberClusters >= 1):
		print_number_clusters(memberList)
	count_tentative_sucess		= 0
	count_tentative_fail		= 0
	count_tentative_already_done	= 0

	while tent_heap.nelem() > 0:
		rec = tent_heap.deleteTop()
		A = rec['pointA']
		B = rec['pointB']
		d = rec['dist']
		if cluster[A] != cluster[B]:
			res = Tentative_Merge(cluster[A], cluster[B], cluster, memberList, tlshList, tobjList, rootVPT, CDist)
			if (res > 0):
				count_tentative_sucess += 1
			else:
				count_tentative_fail   += 1
			# end if
		else:
			count_tentative_already_done	+= 1
		# end if
	# end while
	if (hac_verbose >= 1):
		print("tentative_already_done	=", count_tentative_already_done)
		print("tentative_sucess		=", count_tentative_sucess)
		print("tentative_fail		=", count_tentative_fail)

	if (hac_verbose >= 1) and (ndata >= 1000):
		print_time("End-Step-2", 1)

	##########################
	# Step 3: Find Edge Cases
	##########################
	if (hac_verbose >= 1) or (showNumberClusters >= 1):
		print_number_clusters(memberList)
	if (step3 == 0):
		if (hac_verbose >= 1) and (ndata >= 1000):
			print_time("Not-doing-Step-3")
	else:
		ITERATION = HAC_T_step3(tlshList, tobjList, CDist, rootVPT, memberList, cluster)
		if (ITERATION != 2):
			print("INFO: NOT OPTIMAL CLUSTERING")
		if (hac_verbose >= 1) and (ndata >= 1000):
			print_time("End-Step-3")
	# end if
	if (hac_verbose >= 1) or (showNumberClusters >= 1):
		print_number_clusters(memberList, True)
	printAllCluster(outfname, cenfname, cluster, memberList, tlshList, tobjList, labels, hac_verbose)

	cln = 0
	dbscan_like_cluster = [-1] * len(cluster)
	for ci in range(0, len(memberList) ):
		ml = memberList[ci]
		if (len(ml) > 1):
			for x in ml:
				dbscan_like_cluster[x] = cln
			cln += 1
		# end if
	# end for
	return(dbscan_like_cluster)

def DBSCAN_procedure(fname, CDist, outfname, cenfname, verbose=0):
	(tlist, labels) = tlsh_csvfile(fname)
	res = runDBSCAN(tlist, eps=CDist, min_samples=2)
	outputClusters(outfname, tlist, res.labels_, labels)
	return(res.labels_)

def read_data(fname):
	# print("start fname=", fname)
	(tlshList, labels) = tlsh_csvfile(fname)
	tobjList = []
	for tstr in tlshList:
		h1 = tlsh.Tlsh()
		h1.fromTlshStr(tstr)
		tobjList.append(h1)
	# end for
	# print("end")
	return(tlshList, tobjList, labels)
