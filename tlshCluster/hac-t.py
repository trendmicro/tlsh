from pylib.hac_lib import *

##############################################
# simple_unittests()
##############################################

def simple_unittests():
	(tlshList, tobjList, labels) = read_data(fname)

	tidxList = range(0, len(tlshList) )
	root = VPTGrow(tlshList, tobjList, tidxList)
	root.PrintTree(3,0)

	### id = 9655
	id = 9

	best = { "dist":99999, "point":None, "idx":-1 }
	ignoreList = [ id ]
	searchTlsh = tlshList[ id ]
	searchItem = tlsh.Tlsh()
	searchItem.fromTlshStr(searchTlsh)

	tmpcluster = list(range(0, len(tlshList)))

	print("before: ", best);
	VPTSearch(root, searchItem, id, tmpcluster, id, best)
	print("after:  ", best)

	dist = best['dist']
	B    = best['idx']
	linbest = { "dist":99999, "point":None, "idx":-1 }
	linearSearch(searchItem, tobjList, ignoreList, linbest)
	lindist = linbest['dist']
	linB    = linbest['idx']
	if (lindist < dist):
		print("error: dist=",	 dist,	  "B=", B)
		print("error: lindist=", lindist, "linB=", linB)
		sys.exit()
	# end if

	### heap_tester()

##############################################
# main() program
##############################################

import argparse

parser = argparse.ArgumentParser(prog='hac')
parser.add_argument('-v',       help='verbose',			type=int,	default=0)
parser.add_argument('-opt',     help='opt',			type=int,       default=0)
parser.add_argument('-dbscan',  help='dbscan',			type=int,       default=0)
parser.add_argument('-allow',	help='allow stringy clusters',	type=int,       default=0)
parser.add_argument('-cdist',   help='cdist',			type=int,       default=30)
parser.add_argument('-f',       help='fname',			type=str,       default="")
parser.add_argument('-o',       help='outfname',		type=str,       default="")
parser.add_argument('-oc',      help='out centers fname',	type=str,       default="")
parser.add_argument('-step3',   help='step3',			type=int,       default=1)
parser.add_argument('-showtime',help='showtime',		type=int,       default=0)
parser.add_argument('-showcl',  help='show number clusters',	type=int,       default=0)
parser.add_argument('-utest',   help='unittest',		type=int,       default=0)

args = parser.parse_args()
verbose		= args.v
opt		= args.opt
dbscan		= args.dbscan
allow		= args.allow
CDist		= args.cdist
fname		= args.f
outfname	= args.o
cenfname	= args.oc
step3		= args.step3
showtime	= args.showtime
showcl		= args.showcl
unittest	= args.utest

if (opt == 0):
	linearCheck = False
if (showtime <= 1):
	###############
	# showtime == 0	no timing performed
	# showtime == 1	time the overall program
	# showtime == 2	time parts of the algorithm
	###############
	setNoTiming()
if (showcl):
	setShowNumberClusters()

if (fname == ""):
	print("need a -f fname")
	sys.exit(1)

if (outfname == ""):
	print("need a -o outfname")
	sys.exit(1)

if (unittest > 0):
	simple_unittests()

import datetime
import time
if (showtime == 1):
	startTime	= datetime.datetime.now()
# end if

if (opt == 1):
	step3 = 1
	HAC_T_opt(fname, CDist, step3, outfname, cenfname)
elif (dbscan == 1):
	DBSCAN_procedure(fname, CDist, outfname, cenfname)
elif (allow == 1):
	HAC_T    (fname, CDist, step3, outfname, cenfname, True)
else:
	HAC_T    (fname, CDist, step3, outfname, cenfname, False)

if (showtime == 1):
	endTime		= datetime.datetime.now()
	tdelta = (endTime - startTime)
	delta_micro	= tdelta.microseconds + tdelta.seconds * 1000000
	delta_ms	= int( delta_micro / 1000 )
	print("HAC_T (ms)	"  + fname + "	" + str(delta_ms))
# end if
