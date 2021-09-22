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
# tlsh_csv files
##########################################
import csv

def tlsh_csvfile(fname, verbose=0):
	tlshCol=-1
	hashCol=-1
	lablCol=-1

	tlist=[]
	labelList=[]
	with open(fname) as csv_file:
		csv_reader = csv.reader(csv_file, delimiter=',')
		line_count = 0
		for row in csv_reader:
			if line_count == 0:
				for x in range(len(row)):
					rval = row[x].lower()
					if (rval == 'tlsh'):
						tlshCol = x
					elif (rval == 'sha256') or (rval == 'sha1') or (rval == 'md5') or (rval == 'sha1_hash') or (rval == 'sha256_hash'):
						hashCol = x
					elif (rval == 'signature'):
						#############################
						# signature overrides other label candidates
						#############################
						lablCol = x
					else:
						if (lablCol == -1):
							if (verbose > 0):
								print("using " + rval + " as label")
							lablCol = x
						# end if
					# end if
				# end for
				if (tlshCol == -1):
					print("error: file " + fname + " has no tlsh column: " + str(row) )
					return (None, None)
				# end if
				line_count += 1
			else:
				tlshVal = row[tlshCol]

				if (hashCol != -1):
					hashVal = row[hashCol]
				else:
					hashVal = ""
				# end if

				if (lablCol != -1):
					lablVal = row[lablCol]
				else:
					lablVal = ""
				# end if
				if (lablCol != -1) and (hashCol != -1):
					lab = lablVal + " " + hashVal
					lab = lablVal
				elif (lablCol != -1):
					lab = lablVal
				else:
					lab = hashVal

				okLine = False
				if (len(tlshVal) == 72) and (tlshVal[:2] == "T1"):
					okLine = True
				if (len(tlshVal) == 70):
					okLine = True
				if (okLine):
					tlist.append(tlshVal)
					labelList.append(lab )
				elif (tlshVal != 'TNULL'):
					print("warning. Bad line line=", line_count, " tlshVal=", tlshVal )
				# end if

				line_count += 1
			# end if
		# end for
		if (verbose > 0):
			print(f'Read in {line_count} lines.')
		return(tlist, labelList)
	print("error: could not find file: " + fname)
	return (None, None)

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
	with open(outfname, "w") as f:
		global tptr
		#
		# we call tlist2cdata to that we set tptr
		#
		tdata = tlist2cdata(tlist)
		# cluster is set to None since it is not used (by this program)
		cluster = None
		printAllCluster(f, cluster, members, tlist, tptr, labelList)
		if (not quiet):
			print("written ", outfname)
