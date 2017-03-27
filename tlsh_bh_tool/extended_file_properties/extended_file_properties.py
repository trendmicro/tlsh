import os, sys, pefile, M2Crypto, logging
from M2Crypto import BIO, SMIME, X509, m2

class self:
    logging.config.fileConfig('tlsh_bh_tool.conf')
    logger = logging.getLogger('tlshbh')


def _normalizeString(string):
    try:
        return string.replace(',', ';')
    except Exception, ex:
        return string

def getBasicFileProperties(filename):
    pe = None
    prop_details = {}
    try:
        pe = pefile.PE(filename)
    except Exception, ex:
        self.logger.info("Cannot open PE file : %s" % ex)
    if pe != None:
        try:
            for f in pe.FileInfo:
                if str(f.Key) == 'StringFileInfo':
                    for e in f.StringTable[0].entries:
                        prop_details[str(e).replace(" ", "").strip()] = _normalizeString(str(unicode(f.StringTable[0].entries[e]).encode('ascii', 'ignore')).strip())
                    break
        except Exception, ex:
            self.logger.info("File has no file properties")
        pe.close()
        del pe
        return prop_details

def getCertificateDetails(filename):
    basename = ""
    cert_details = {}
    basename = os.path.basename(filename).split('.')[0]
    cert_name = "%s_digisig.crt" % basename
    try:
        if _extractDigitalSignature(filename, signatureFile=cert_name) != None:
            bio=BIO.File(open(cert_name))
            smime_object = SMIME.PKCS7(m2.pkcs7_read_bio_der(bio._ptr()))
            signers = smime_object.get0_signers(X509.X509_Stack())
            cert_details["cert_issued_by"] = signers[0].get_issuer().CN
            cert_details["cert_issued_to"] = signers[0].get_subject().CN
            validity = signers[0].get_not_after().get_datetime()
            cert_details["cert_expiration"] = "%s-%s-%s" % (validity.year, validity.month, validity.day)
            bio.close()
            del bio
    except Exception, ex:
        print "ERROR: Problem in retrieving certificate details : %s" % ex
    finally:
        if os.path.exists(cert_name): os.unlink(cert_name)
        return cert_details

def _extractDigitalSignature(signedFile, signatureFile=None):
    """Extracts the digital signature from file SignedFile
       When SignatureFile is not None, writes the signature to SignatureFile
       Returns the signature
    """
    try:
        pe =  pefile.PE(signedFile)
        address = pe.OPTIONAL_HEADER.DATA_DIRECTORY[pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_SECURITY']].VirtualAddress
        size = pe.OPTIONAL_HEADER.DATA_DIRECTORY[pefile.DIRECTORY_ENTRY['IMAGE_DIRECTORY_ENTRY_SECURITY']].Size
        if address == 0:
            print '[STATUS]: Source file not signed'
            return
        signature = pe.write()[address+8:]
        if signatureFile:
            f = file(signatureFile, 'wb+')
            f.write(signature)
            f.close()
        return signature
    except Exception, ex:
        print ex
        return None

