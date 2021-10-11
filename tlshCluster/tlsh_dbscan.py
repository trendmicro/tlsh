from pylib.hac_lib import *

##############################################
# main() program
##############################################

CDist = 30
fname = ""
outfname = ""

import argparse

parser = argparse.ArgumentParser(prog='hac')
parser.add_argument('-cdist',   help='cdist',			type=int,       default=30)
parser.add_argument('-f',       help='fname',			type=str,       default="")
parser.add_argument('-o',       help='outfname',		type=str,       default="")
parser.add_argument('-oc',      help='out centers fname',	type=str,       default="")
parser.add_argument('-showtime',help='showtime',		type=int,       default=0)
parser.add_argument('-showcl',  help='show number clusters',	type=int,       default=0)

args		= parser.parse_args()
CDist		= args.cdist
fname		= args.f
outfname	= args.o
cenfname	= args.oc
showtime	= args.showtime
showcl		= args.showcl

if (showcl):
	setShowNumberClusters()

if (fname == ""):
	print("need a -f fname")
	sys.exit(1)

if (outfname == ""):
	print("need a -o outfname")
	sys.exit(1)

import datetime
import time
if (showtime == 1):
	startTime	= datetime.datetime.now()
# end if

DBSCAN_procedure(fname, CDist, outfname, cenfname)

if (showtime == 1):
	endTime		= datetime.datetime.now()
	tdelta = (endTime - startTime)
	delta_micro	= tdelta.microseconds + tdelta.seconds * 1000000
	delta_ms	= int( delta_micro / 1000 )
	print("DBSCAN (ms)	"  + fname + "	" + str(delta_ms))
# end if
