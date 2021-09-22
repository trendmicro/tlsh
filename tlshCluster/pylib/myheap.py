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

# from https://www.section.io/engineering-education/heap-data-structure-python/

class MinHeap:
	def __init__(self):
		# Initialize a heap using list
		self.heap = []

	def nelem(self):
		# The parent is located at floor((i-1)/2)
		return(len(self.heap))

	def getParentPosition(self, i):
		# The parent is located at floor((i-1)/2)
		return int((i-1)/2)

	def getLeftChildPosition(self, i):
		# The left child is located at 2 * i + 1
		return 2*i+1

	def getRightChildPosition(self, i):
		# The right child is located at 2 * i + 2
		return 2*i+2

	def hasParent(self, i):
		# This function checks if the given node has a parent or not
		return self.getParentPosition(i) < len(self.heap)

	def hasLeftChild(self, i):
		# This function checks if the given node has a left child or not
		return self.getLeftChildPosition(i) < len(self.heap)

	def hasRightChild(self, i):
		# This function checks if the given node has a right child or not
		return self.getRightChildPosition(i) < len(self.heap)

	def insert(self, key, dist):
		rec = { "key":key, "dist":dist }
		self.heap.append(rec) # Adds the key to the end of the list
		self.heapify(len(self.heap) - 1) # Re-arranges the heap to maintain the heap property

	def dist(self, i):
		lenheap = len(self.heap)
		if (i >= lenheap):
			return(9999999)
		rec = self.heap[i]
		return( rec['dist'] )

	def deleteTop(self):
		lenheap = len(self.heap)
		if (lenheap == 0):
			return(None)
		rec = self.heap[0]
		self.heap[0] = self.heap[lenheap-1]
		self.heap.pop()
		self.heapify2(0)
		return rec['key'] # Returns the min value in the heap in O(1) time.

	def heapify(self, i):
		while(self.hasParent(i) and self.dist(i) < self.dist(self.getParentPosition(i))): # Loops until it reaches a leaf node
			self.heap[i], self.heap[self.getParentPosition(i)] = self.heap[self.getParentPosition(i)], self.heap[i] # Swap the values
			i = self.getParentPosition(i) # Resets the new position

	def heapify2(self, i):
		keepgoing = 1
		thisval = self.dist(i)
		lc = self.dist(self.getLeftChildPosition(i))
		rc = self.dist(self.getRightChildPosition(i))
		while (thisval > lc) or (thisval > rc):
			if (lc < rc):
				pos = self.getLeftChildPosition(i)
			else:
				pos = self.getRightChildPosition(i)
			# end if
			# Swap the values
			self.heap[i], self.heap[pos] = self.heap[pos], self.heap[i]
			i = pos
			lc = self.dist(self.getLeftChildPosition(i))
			rc = self.dist(self.getRightChildPosition(i))

	def printHeap(self):
		print(self.heap) # Prints the heap

def heap_tester():
	heap=MinHeap()
	heap.insert("apple", 0)
	heap.insert("b", 6)
	heap.insert("c", 3)
	heap.insert("d", 46)
	heap.printHeap()

	while (heap.nelem() > 0):
		top = heap.deleteTop()
		print("top=", top)
	heap.printHeap()
