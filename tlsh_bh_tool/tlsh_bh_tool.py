__name__ = "tlsh_bh_tool"
__version__ = 1.0
__author__ = "Jayson Pryde"

import os, sys, tlsh, argparse, threading, pefile, requests
import logging, logging.config, hashlib, ConfigParser, simplejson
from extended_file_properties import extended_file_properties as efp
from os.path import getsize

class TlshStruct:
        files = []
        threads = []
        flag = True
        out = None
        lock = None
        logger = None
        outname = None
        counter = int(0)
        thread_count = int(0)
        restrict = None
        query_url = None
        file_basic_details =  {}
        file_prop_details = {}
        file_cert_details = {}
        

def main(options):
        if _init(options):
                if os.path.isfile(options.file): _processFile1(options.file)
                elif os.path.isdir(options.file):
                        _enumerateFiles(options.file)
                        _initOut()
                        _initScanningThreads()
                        _startScanningThreads()
                        _stopScanningThreads()
                        _deinitOut()

def _enumerateFiles(folder_path):
        for root, dirs, files in os.walk(folder_path):
                for name in files: TlshStruct.files.append(os.path.join(root, name))

def _init(options):
        ret = True
        try:
                TlshStruct.thread_count = int(options.thread_count)
                logging.config.fileConfig('tlsh_bh_tool.conf')
                TlshStruct.logger = logging.getLogger('tlshbh')
                cfg = ConfigParser.ConfigParser()
                cfg.readfp(open("./tlsh_bh_tool.cfg"))
                TlshStruct.file_basic_details["apikey"] = cfg.get('Credentials', 'apikey')
                TlshStruct.file_basic_details["user"] = cfg.get('Credentials', 'user')
                TlshStruct.query_url = cfg.get('Webservice', 'query_url')
                TlshStruct.restrict = int(options.restrict)
                TlshStruct.outname = options.out
        except Exception, ex:
                print "ERROR: Problem during initialization : %s" % ex
                ret = False
        finally:
                return ret

def _initOut():
        try:
                TlshStruct.out = open(TlshStruct.outname, "w")
        except Exception, ex:
                print "ERROR: Problem initializing output : %s" % ex

def _deinitOut():
        if TlshStruct.out is not None: TlshStruct.out.close()

def _initScanningThreads():
        try:
                TlshStruct.lock = threading.Lock()
		for i in range(0, TlshStruct.thread_count): TlshStruct.threads.append(threading.Thread(target=_processFile2))
	except Exception, ex:
                print "ERROR: Problem in initializing scanning threads : %s" % ex

def _startScanningThreads():
        for thr in TlshStruct.threads: thr.start()

def _stopScanningThreads():
        for thr in TlshStruct.threads:
                if thr.isAlive(): thr.join()

def _getSha256(filename):
        h = None
        sha256 = "NULL"
        try:
                h = hashlib.sha256()
                with open(filename, "rb") as f:
                        while True:
                                block = f.read(2**12)
                                if not block: break
                                h.update(block)
                        sha256 = str(h.hexdigest())
        except Exception, ex:
                TlshStruct.logger.error("Problem in getting SHA1 of %s : %s" % (filename, ex))
                sha256 = "ERROR"
        finally:
                return sha256

def _sendQuery():
        result = {}
        params = dict(TlshStruct.file_basic_details.items() + TlshStruct.file_prop_details.items() + TlshStruct.file_cert_details.items())
        try:
                response = requests.get(TlshStruct.query_url, params=params, verify=False)
                if response.status_code == 200: result = simplejson.loads(response.content)
        except Exception, ex:
                TlshStruct.logger.error("Problem in sending query : %s" % ex)
        finally:
                return result

def _resetFileDetails():
        TlshStruct.file_basic_details["tlsh"] = ""
        TlshStruct.file_basic_details["sha256"] = ""
        TlshStruct.file_prop_details = {}
        TlshStruct.file_cert_details = {}

def _recordResults(result):
    if result is None: return
    if result["status"] == "ok":
        if len(result["matches"]) != 0:
            if TlshStruct.out is None:
                try:
                    f = open(TlshStruct.outname, "w")
                    for r in result["matches"]:
                        f.write("%s,%s,%s,%s,%s\n" % (TlshStruct.file_basic_details["sha256"], TlshStruct.file_basic_details["tlsh"],
                                                  r["id"], r["tag"], r["distance_score"]))
                    f.close()
                except Exception, ex:
                    TlshStruct.logger.error("Cannot write file %s : %s" %  (TlshStruct.outname, ex))
            else:
                for r in result["matches"]:
                    TlshStruct.out.write("%s,%s,%s,%s,%s\n" % (TlshStruct.file_basic_details["sha256"], TlshStruct.file_basic_details["tlsh"], 
                                                              r["id"], r["tag"], r["distance_score"]))

def _processFile1(filename):
        print "Processing %s..." % filename
        _resetFileDetails()
        if getsize(filename) <= 512: TlshStruct.logger.error("File %s too small to compute tlsh value")
        else:
                result = None
                try:
                        TlshStruct.file_basic_details["filename"] = filename
                        TlshStruct.file_basic_details["tlsh"] = tlsh.hash(open(filename, "rb").read())
                        TlshStruct.file_basic_details["sha256"] = _getSha256(filename)
                        if not TlshStruct.restrict:
                                prop_details = efp.getBasicFileProperties(filename)
                                cert_details = efp.getCertificateDetails(filename)
                                TlshStruct.file_prop_details = prop_details if prop_details is not None else {}
                                TlshStruct.file_cert_details = cert_details if cert_details is not None else {}
                                result = _sendQuery()
                except Exception, ex:
                        print "ERROR: Problem in getting tlsh value of %s : %s" % (filename, ex)
                        tlsh_val = "error"
                finally:
                        _recordResults(result)

def _processFile2():
        while TlshStruct.flag:
                TlshStruct.lock.acquire()
                try:
                        filename = TlshStruct.files.pop()
                        _processFile1(filename)
                except Exception, ex:
                        if TlshStruct.counter == TlshStruct.thread_count:
                                TlshStruct.flag = False
                                TlshStruct.counter = 0
                        else: TlshStruct.counter+=1
                finally:
                        TlshStruct.lock.release()

def _showBanner():
	if os.name == "nt": os.system("cls")
	elif os.name == "posix": os.system("clear")
	print "********************************"
	print "*   TLSH BlackHat Tool v1.0    *"
	print "*         Demo Version         *"
	print "********************************"

if __name__ == "tlsh_bh_tool":
	_showBanner()
	parser = argparse.ArgumentParser()
	parser.add_argument("file", help="directory containing files | file")
	parser.add_argument("-out", default="matches.csv", help="CSV file containing query results. Default is matches.csv")
	parser.add_argument("-restrict", dest="restrict", default=0, help="0 == send all file properties | 1 == send only basic information (i.e. sha256 and tlsh).")
	parser.add_argument("-tc", dest="thread_count", default=3, help="scanning thread count. Default is 3")
	options = parser.parse_args()
	main(options)

