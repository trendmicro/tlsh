import sys
import tlsh

def main(argv):
    nargs=len(argv)
    if nargs != 3:
        print("usage: tlsh_parts.py -d digest")
        sys.exit()
    # end if
    digest		= ""
    if argv[1] == "-d":
        digest=argv[2]
    # end if
    if (digest == ""):
        print("usage: tlsh_parts.py -d digest")
        sys.exit()
    # end if

    h1 = tlsh.Tlsh()
    try:
        h1.fromTlshStr(digest)
    except:
        print("invalid digest: " + digest)
        sys.exit()
    # end try

    print("checksum:	" + str(h1.checksum(0)))
    print("lvalue:	" + str(h1.lvalue))
    print("q1ratio:	" + str(h1.q1ratio))
    print("q2ratio:	" + str(h1.q2ratio))
    for bi in range(0,128):
        print("bucket " + str(bi) + ":	" + str(h1.bucket_value(bi)))
    # end for

main(sys.argv)
