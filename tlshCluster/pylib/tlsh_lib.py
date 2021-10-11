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

import tlsh
import numpy as np

from pylib.printCluster import *

##########################################
# creating tlsh pairwise distance for scipy and sklearn
##########################################

# https://stackabuse.com/hierarchical-clustering-with-python-and-scikit-learn/

from scipy.spatial.distance import pdist

verbose=0
tptr=[]

nDistCalc=0

def resetDistCalc():
	global nDistCalc
	nDistCalc=0

def lookupDistCalc():
	global nDistCalc
	return(nDistCalc)

def sim(idx1, idx2):
	global tptr
	global nDistCalc
	nDistCalc += 1

	# print("idx1=", idx1)
	# print("idx2=", idx2)
	h1 = tptr[int(idx1[0])]
	h2 = tptr[int(idx2[0])]
	dist=h1.diff(h2)
	return dist

def tlist2cdata(tlist):
	global tptr
	tptr=[]
	tidx = []
	idx=0
	for x in tlist:
	    h1 = tlsh.Tlsh()
	    h1.fromTlshStr(x)
	    tptr.append(h1)
	    elem=[ idx ]
	    tidx.append(elem)
	    idx += 1
	# end for 
	tdata = np.array(tidx)
	return(tdata)

##########################################
# tlsh_dendrogram
##########################################

from scipy.cluster.hierarchy import dendrogram, linkage
from matplotlib import pyplot as plt

def tlsh_dendrogram(tlist, labelList=None):
	if (len(tlist) < 2):
		print("The list of tlsh is too short. len(tlist)=", len(tlist) )
		print("No dendrogram can be built.")
		return
	# end if
	if (len(tlist) >= 100):
		print("warning: The list of TLSH values is too long to show a sensible dendrogram.")
		print("It is recommended that you filter to a smaller list of TLSH values.")
		print()
	# end if
	tdata = tlist2cdata(tlist)
	Y = pdist(tdata, sim)

	linked = linkage(Y, 'single')
	if (labelList is None):
		labelList = range(1, len(tdata)+1)
	plt.figure(figsize=(15, 9))
	dendrogram(linked,
		orientation='left',
		labels=labelList,
		distance_sort='descending',
		show_leaf_counts=True)
	plt.show()

##########################################
# show a malware bazaar cluster
##########################################
def mb_show_sha1(family, thisDate=None, nitems=None, fname="malbaz/clust_389300.csv", showN=10, showC=1):
	(tlist, labels) = tlsh_csvfile(fname, searchColName="family", searchValueList=[family], sDate=thisDate, eDate=thisDate, searchNitems=nitems)
	if (tlist is None):
		return
	if (len(tlist) == 0):
		print("found no cluster")
	elif (len(tlist) <= showC):
		for cenTlsh in tlist:
			print("cluster with cenTlsh=" + cenTlsh)
			fullmb = "malbaz/mb_full.csv"
			(tlist2, labels2) = tlsh_csvfile(fullmb, simTlsh=cenTlsh, simThreshold=30)
			if (tlist2 is None):
				print("you need to run the script process_mb.sh in malbaz")
				return
			nfound = len(tlist2)
			if (nfound > showN):
				print("showing first ", showN, " samples")
				print("	increase the showN parameter to show more..." )
				nfound = showN
			# end if
			labList  = labels2[0]
			dateList = labels2[1]
			hashList = labels2[2]
			for idx in range(nfound):
				# print(tlist2[idx] + "\t" + labList[idx] + "\t" + dateList[idx] + "\t" + hashList[idx] )
				print(hashList[idx] )
			# end for
		# end for
	else:
		print("found ", len(tlist), " clusters.")
		print("Use parameters 'thisDate' and 'nitems' to uniquely specify cluster")
		print("OR")
		print("set showC parameter to show more clusters")
	# end if

##########################################
# tlsh_csv files
##########################################
import csv

def tlsh_csvfile(fname, searchColName=None, searchValueList=None, simTlsh=None, simThreshold=150, sDate=None, eDate=None, searchNitems=None, verbose=0):
	tlshCol=-1
	hashCol=-1
	lablCol=-1
	timeCol=-1
	othCol=-1
	srchCol=-1
	itemCol=-1

	tlist=[]
	labelList=[]
	dateList =[]
	hashList =[]
	addSampleFlag = True

	if (simTlsh is not None) and (simThreshold == 150):
		print("using default simThreshold=150")

	# make all lower case so that we catch inconsistencies in the use of case
	if (searchValueList is not None):
		searchValueList = [s.lower() for s in searchValueList]

	try:
		csv_file = open(fname)
	except:
		print("error: could not find file: " + fname)
		return (None, None)
	# end try/catch

	csv_reader = csv.reader(csv_file, delimiter=',')
	line_count = 0
	for row in csv_reader:
		if line_count == 0:
			for x in range(len(row)):
				rval = row[x].lower()
				if (searchColName is not None) and (searchColName.lower() == rval):
					srchCol = x
				# end if
				if (rval == 'tlsh'):
					tlshCol = x
				elif (rval == 'sha256') or (rval == 'sha1') or (rval == 'md5') or (rval == 'sha1_hash') or (rval == 'sha256_hash'):
					hashCol = x
				elif (rval == 'signature') or (rval == 'label'):
					#############################
					# signature overrides other label candidates
					#############################
					if (lablCol != -1):
						print("warning: found both 'signature' column and 'label' column")
						print("using ", row[lablCol] )
					else:
						lablCol = x
					# end if
				elif (rval == 'first_seen_utc') or (rval == 'firstseen'):
					timeCol = x
				elif (rval == 'nitems'):
					itemCol = x
				else:
					if (othCol == -1):
						othCol = x
				# end if
			# end for
			if (lablCol == -1) and (othCol != -1):
				if (verbose > 0):
					print("using " + row[othCol] + " as label")
				# end if
				lablCol = othCol
			# end if

			if (tlshCol == -1):
				print("error: file " + fname + " has no tlsh column: " + str(row) )
				return (None, None)
			# end if
			line_count += 1
		else:
			tlshVal = row[tlshCol]
			hashVal = row[hashCol] if (hashCol != -1) else ""
			lablVal = row[lablCol] if (lablCol != -1) else ""
			srchVal = row[srchCol] if (srchCol != -1) else ""
			itemVal = row[itemCol] if (itemCol != -1) else ""

			if (timeCol != -1):
				ts	= row[timeCol]
				# first_seen_utc (in malware bazaar) takes format "2021-09-17 06:39:44"
				# we want the first 10 characters
				dateVal = ts[:10]
			else:
				dateVal = ""
			# end if

			if (lablCol != -1) and (hashCol != -1):
				lab = lablVal + " " + hashVal
				lab = lablVal
			elif (lablCol != -1):
				lab = lablVal
			else:
				lab = hashVal
			# end if

			#####################
			# check line OK
			#####################
			okLine		= False
			if (len(tlshVal) == 72) and (tlshVal[:2] == "T1"):
				okLine = True
			if (len(tlshVal) == 70):
				okLine = True

			if (okLine):
				#####################
				# check search criteria
				#####################
				includeLine	= True
				if (srchVal != "") and (searchValueList is not None):
					if (srchVal.lower() not in searchValueList):
						includeLine	= False
					# end if
				# end if
				if (simTlsh is not None):
					h1 = tlsh.Tlsh()
					h1.fromTlshStr(simTlsh)
					h2 = tlsh.Tlsh()
					h2.fromTlshStr(tlshVal)
					dist=h1.diff(h2)
					if (dist > simThreshold):
						includeLine	= False
					elif (dist == 0):
						# the search query is an item in our file
						#	so modify the label
						#	and do not add the Query
						addSampleFlag = False
						lab = "QUERY " + lab
					# end if
				# end if
				#####################
				# check date range
				#####################
				if (sDate is not None) and (dateVal != ""):
					if (dateVal < sDate):
						includeLine	= False
					# end if
				# end if
				if (eDate is not None) and (dateVal != ""):
					# print("check dateVal=", dateVal, " eDate=", eDate)
					if (dateVal > eDate):
						includeLine	= False
					# end if
				# end if
				#####################
				# check item value
				#####################
				if includeLine and (searchNitems is not None) and (itemVal != ""):
					if (itemVal != str(searchNitems)):
						includeLine	= False
					# end if
				# end if
				if (includeLine):
					tlist.append(tlshVal)
					labelList.append(lab)
					dateList .append(dateVal)
					hashList .append(hashVal)
				# end if
			elif (tlshVal not in ["TNULL", "", "n/a"]):
				print("warning. Bad line line=", line_count, " tlshVal=", tlshVal )
			# end if

			line_count += 1
		# end if
	# end for
	if (verbose > 0):
		print(f'Read in {line_count} lines.')
	if (simTlsh is not None) and (addSampleFlag):
		tlist.append(simTlsh)
		labelList.append("QUERY")
		dateList .append("")
		hashList .append("")
	# end if
	return(tlist, [labelList, dateList, hashList])

##########################################
# assign clusters to points using sklearn
##########################################

from sklearn.metrics import pairwise_distances
from sklearn.cluster import AgglomerativeClustering

def sim_affinity(tdata):
	return pairwise_distances(tdata, metric=sim)

def assignCluster(tlist, n_clusters):
	cluster = AgglomerativeClustering(n_clusters=n_clusters, affinity=sim_affinity, linkage='average')
	tdata = tlist2cdata(tlist)
	ClusterNumber=cluster.fit_predict(tdata)
	return(ClusterNumber)
# print(res)

def selectCluster(tlist, clusterNumber, clusterIdx, labelList=None):
	if clusterIdx not in clusterNumber:
		print("clusterIdx=" + str(clusterIdx) + " does not exist")
		return(None, None)
	# end if
	cl_tlist = []
	cl_llist = []
	for xi in range(len(tlist)):
		if (clusterNumber[xi] == clusterIdx):
			cl_tlist.append(tlist[xi])
			if (labelList is None):
				cl_llist.append("cluster" + str(clusterIdx))
			else:
				cl_llist.append(labelList[xi])
			# end if
		# end if
	# end for
	return(cl_tlist, cl_llist)

##########################################
# DBSCAN
##########################################

from sklearn.cluster import DBSCAN

def runDBSCAN(tlist, eps, min_samples, algorithm='auto'):
	tdata = tlist2cdata(tlist)
	res = DBSCAN(eps=eps, min_samples=min_samples, metric=sim, algorithm=algorithm).fit(tdata)
	return(res)
# print(res)

def analyse_clusters(clusterNumber):
	nclusters = max(clusterNumber)
	members = []
	for x in range(nclusters+1):
		emptyList = []
		members.append(emptyList)
	print(members)
	print(len(members))
	noise = []
	for idx in range(len(clusterNumber)):
		cln = clusterNumber[idx]
		if (cln != -1):
			members[cln].append(idx)
		else:
			noise.append(idx)
		# end if
	# end for
	for x in range(nclusters):
		print("cluster " + str(x) )
		print(members[x])
	# end for
	print("noise: ")
	print(noise)

def outputClusters(outfname, tlist, clusterNumber, labelList, quiet=False):
	nclusters = max(clusterNumber)
	members = []
	for x in range(nclusters+1):
		emptyList = []
		members.append(emptyList)
	# print(members)
	# print(len(members))
	noise = []
	for idx in range(len(clusterNumber)):
		cln = clusterNumber[idx]
		if (cln != -1):
			members[cln].append(idx)
		else:
			noise.append(idx)
		# end if
	# end for


	global tptr
	#
	# we call tlist2cdata to that we set tptr
	#
	tdata = tlist2cdata(tlist)
	# cluster is set to None since it is not used (by this program)
	cluster = None
	cenfname = ""
	verbose = 0
	if (not quiet):
		verbose = 1
	printAllCluster(outfname, cenfname, cluster, members, tlist, tptr, labelList, verbose)
