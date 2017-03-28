import os, sys, requests, argparse, ConfigParser
import logging, logging.config

class self:
    logger = None
    register_url = None

def main(options):
    if _init(): _register(options)

def _init():
    ret = True
    try:
        logging.config.fileConfig('tlsh_reg.conf')
        self.logger = logging.getLogger('tlshreg')
        cfg = ConfigParser.ConfigParser()
        cfg.readfp(open("./tlsh_reg.cfg"))
        self.register_url = cfg.get('Webservice', 'register_url')
    except Exception, ex:
        print "[ERROR]: Problem in initialization : %s" % ex
        ret = False
    finally:
        return ret

def _register(options):
    resp = None
    try:
        data = {"first_name":options.first_name, "last_name":options.last_name,
                "email":options.email}
        resp = requests.get(self.register_url, params=data).content
        self.logger.info(resp)
    except Exception, ex:
        self.logger.error("Problem in registering : %s" % ex)

def _showBanner():
    if os.name == "nt": os.system("cls")
    elif os.name == "posix": os.system("clear")
    print "*******************************"
    print "*   TLSH Register Tool v1.0   *"
    print "*******************************"

if __name__ == "__main__":
    _showBanner()
    parser = argparse.ArgumentParser()
    parser.add_argument("first_name", help="first name")
    parser.add_argument("last_name", help="last name")
    parser.add_argument("email", help="email")
    options = parser.parse_args()
    main(options)
