/*
 * Dibbler - a portable DHCPv6
 *
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>
 *          Marek Senderski <msend@o2.pl>
 * changes: Krzysztof Wnuk <keczi@poczta.onet.pl>
 *          Micha� Kowalczuk <michal@kowalczuk.eu>
 *          Grzegorz Pluto <g.pluto(at)u-r-b-a-n(dot)pl>
 *
 * released under GNU GPL v2 only licence
 *
 * $Id: AddrMgr.cpp,v 1.37 2008-08-29 00:07:26 thomson Exp $
 *
 */

#include <iostream>
#include <fstream>
#include "Portable.h"
#include <limits.h>
#include "AddrMgr.h"
#include "AddrClient.h"
#include "DHCPConst.h"
#include "Logger.h"

TAddrMgr::TAddrMgr(string xmlFile, bool loadfile)
{
    this->IsDone = false;
    this->XmlFile = xmlFile;
    
    if (loadfile) {
      	dbLoad(xmlFile.c_str());
    } else {
	      Log(Debug) << "Skipping database loading." << LogEnd;
    }
    DeleteEmptyClient = true;
}

/**
 * @brief loads XML database from disk
 *
 * this method loads database from disk. (see dump() method 
 * for storing the database on disk). Right now it is used by
 * client only, but it can be adapted easily for server side.
 * After DB is loaded, necessary TAddrClient, TAddrIA and TAddrAddr
 * lists are created.
 *
 * There are 2 versions of this function:
 * 1. libxml2 based. Libxml2 is an external library that provides
 *  xml parsing capabilities. It is reliable, but adds big dependency
 *  to dibbler, therefore it is not currently used.
 * 2. internal parser. See xmlLuadBuiltIn() method. It is very small,
 *  works with files generated by dibbler, but due to its size it is
 *  quite dumb and may be confused quite easily.
 *
 * @param xmlFile filename of the database
 *
 */
void TAddrMgr::dbLoad(const char * xmlFile)
{
#ifdef MOD_LIBXML2
    Log(Debug) << "Loading " << xmlFile << " (using libxml2)." << LogEnd;
    xmlDocPtr root;
    root = xmlLoad(xmlFile);
    if (!root) {
 	Log(Error) << "File loading has failed." << LogEnd;
	return;
    }
    this->parseAddrMgr(root,0);
    xmlFreeDoc(root);
#else
    Log(Info) << "Loading old address database (" << xmlFile << "), using built-in routines." << LogEnd;
    xmlLoadBuiltIn(xmlFile);
#endif
}

/**
 * @brief stores content of the AddrMgr database to a file
 *
 * stores content of the AddrMgr database to XML file
 *
 *
 */
void TAddrMgr::dump()
{
    std::ofstream xmlDump;
    xmlDump.open(this->XmlFile.c_str(), ios::ate);
    xmlDump << *this;
    xmlDump.close();
}

void TAddrMgr::addClient(SPtr<TAddrClient> x)
{
    ClntsLst.append(x);
}

void TAddrMgr::firstClient()
{
    ClntsLst.first();
}

SPtr<TAddrClient> TAddrMgr::getClient()
{
    return ClntsLst.get();
}

/**
 * @brief returns client with a specified DUID
 *
 * returns client with a specified DUID
 *
 * @param duid client DUID 
 *
 * @return smart pointer to the client (or 0 if client is not found)
 */
SPtr<TAddrClient> TAddrMgr::getClient(SPtr<TDUID> duid)
{
    SPtr<TAddrClient> ptr;
    ClntsLst.first();
    while (ptr = ClntsLst.get() ) 
    {
        if ( *(ptr->getDUID()) == (*duid) )
            return ptr;
    }
    return 0;
}

/**
 * @brief returns client with specified SPI index
 *
 * returns client with specified SPI (Security Prameters Index).
 * Useful for security purposes only
 *
 * @param SPI security 
 *
 * @return smart pointer to the client (or 0 if client is not found)
 */
SPtr<TAddrClient> TAddrMgr::getClient(uint32_t SPI)
{
    SPtr<TAddrClient> ptr;
    ClntsLst.first();
    while (ptr = ClntsLst.get() ) 
    {
        if ( ptr->getSPI() == SPI )
            return ptr;
    }
    return 0;
}

/**
 * @brief returns client that leased specified address
 *
 * returns client that leased specified address
 *
 * @param leasedAddr address leased to the client
 *
 * @return smart pointer to the client (or 0 if client is not found)
 */
SPtr<TAddrClient> TAddrMgr::getClient(SPtr<TIPv6Addr> leasedAddr)
{
    SPtr<TAddrClient> cli;
    ClntsLst.first();
    while (cli = ClntsLst.get() ) 
    {
	      SPtr<TAddrIA> ia;
	      cli->firstIA();
	      while (ia = cli->getIA()) {
	          if ( ia->getAddr(leasedAddr) )
		            return cli;
	      }
    }
    return 0;
}

int TAddrMgr::countClient()
{
    return ClntsLst.count();
}

bool TAddrMgr::delClient(SPtr<TDUID> duid)
{
    SPtr<TAddrClient> ptr;
    ClntsLst.first();

    while ( ptr = ClntsLst.get() ) 
    {
        if  ((*ptr->getDUID())==(*duid)) 
        {
            ClntsLst.del();
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------------------
// --- time related methods -------------------------------------------
// --------------------------------------------------------------------

unsigned long TAddrMgr::getT1Timeout()
{
    unsigned long ts = ULONG_MAX;
    SPtr<TAddrClient> ptr;
    ClntsLst.first();
    while (ptr = ClntsLst.get() ) {
        if (ts > ptr->getT1Timeout() )
            ts = ptr->getT1Timeout();
    }
    return ts;
}

unsigned long TAddrMgr::getT2Timeout()
{
    unsigned long ts = ULONG_MAX;
    SPtr<TAddrClient> ptr;
    ClntsLst.first();
    while (ptr = ClntsLst.get() ) {
        if (ts > ptr->getT2Timeout() )
            ts = ptr->getT2Timeout();
    }
    return ts;
}

unsigned long TAddrMgr::getPrefTimeout()
{
    unsigned long ts = ULONG_MAX;
    SPtr<TAddrClient> ptr;
    ClntsLst.first();
    while (ptr = ClntsLst.get() ) {
        if (ts > ptr->getPrefTimeout() )
            ts = ptr->getPrefTimeout();
    }
    return ts;
}

unsigned long TAddrMgr::getValidTimeout()
{
    unsigned long ts = ULONG_MAX;
    SPtr<TAddrClient> ptr;
    ClntsLst.first();
    while (ptr = ClntsLst.get() ) {
        if (ts > ptr->getValidTimeout() )
            ts = ptr->getValidTimeout();
    }
    return ts;
}

/* Prefix Delegation-related method starts here */

/** 
 * adds prefix for a client. If client's IA is missing, add it, too.
 * 
 * @param clntDuid client DUID
 * @param clntAddr client address
 * @param iface    interface index used for client communication
 * @param IAID     IA identifier
 * @param T1       T1 timer value
 * @param T2       T2 timer value
 * @param prefix   prefix to be added
 * @param pref     preferred lifetime
 * @param valid    valid lifetime
 * @param length   prefix length
 * @param quiet    should it be added quietly? (i.e. no messages printed)
 * 
 * @return true if adding was successful
 */
bool TAddrMgr::addPrefix(SPtr<TDUID> clntDuid , SPtr<TIPv6Addr> clntAddr,
                         int iface, unsigned long IAID, unsigned long T1, unsigned long T2, 
                         SPtr<TIPv6Addr> prefix, unsigned long pref, unsigned long valid,
                         int length, bool quiet) {
    // find this client
    SPtr <TAddrClient> ptrClient;
    this->firstClient();
    while ( ptrClient = this->getClient() ) {
        if ( (*ptrClient->getDUID()) == (*clntDuid) ) 
            break;
    }

    // have we found this client? 
    if (!ptrClient) {
	if (!quiet) Log(Debug) << "Adding client (DUID=" << clntDuid->getPlain()
			       << ") to addrDB." << LogEnd;
	ptrClient = new TAddrClient(clntDuid);
	this->addClient(ptrClient);
    }
    return addPrefix(ptrClient, clntDuid, clntAddr, iface, IAID, T1, T2, prefix, pref, valid, length, quiet);
}

bool TAddrMgr::addPrefix(SPtr<TAddrClient> client, SPtr<TDUID> duid , SPtr<TIPv6Addr> addr,
			 int iface, unsigned long IAID, unsigned long T1, unsigned long T2, 
			 SPtr<TIPv6Addr> prefix, unsigned long pref, unsigned long valid,
			 int length, bool quiet) {
    if (!prefix) {
	Log(Error) << "Attempt to add null prefix failed." << LogEnd;
	return false;
    }

    if (!client) {
	Log(Error) << "Unable to add prefix, client not defined." << LogEnd;
	return false;
    }

    // find this PD
    SPtr <TAddrIA> ptrPD;
    client->firstPD();
    while ( ptrPD = client->getPD() ) {
        if ( ptrPD->getIAID() == IAID)
            break;
    }

    // have we found this PD?
    if (!ptrPD) {
	ptrPD = new TAddrIA(iface, TAddrIA::TYPE_PD, addr, duid, T1, T2, IAID);
	client->addPD(ptrPD);
	if (!quiet)
	    Log(Debug) << "PD: Adding PD (iaid=" << IAID << ") to addrDB." << LogEnd;
    }

    SPtr <TAddrPrefix> ptrPrefix;
    ptrPD->firstPrefix();
    while ( ptrPrefix = ptrPD->getPrefix() ) {
        if (*ptrPrefix->get()==*prefix)
            break;
    }

    // address already exists
    if (ptrPrefix) {
        Log(Warning) << "PD: Prefix " << *ptrPrefix
		     << " is already assigned to this PD." << LogEnd;
        return false;
    }

    // add address
    ptrPD->addPrefix(prefix, pref, valid, length);
    if (!quiet)
	Log(Debug) << "PD: Adding " << prefix->getPlain() 
		   << " prefix to PD (iaid=" << IAID 
		   << ") to addrDB." << LogEnd;
    return true;
}

bool TAddrMgr::updatePrefix(SPtr<TDUID> duid , SPtr<TIPv6Addr> addr,
			    int iface, unsigned long IAID, unsigned long T1, unsigned long T2,
			    SPtr<TIPv6Addr> prefix, unsigned long pref, unsigned long valid,
			    int length, bool quiet)
{
    // find client...
    SPtr <TAddrClient> client;
    this->firstClient();
    while ( client = this->getClient() ) {
        if ( (*client->getDUID()) == (*duid) ) 
            break;
    }
    if (!client) {
	Log(Error) << "Unable to update prefix " << prefix->getPlain() << "/" << (int)length << ": DUID=" << duid->getPlain() << " not found." << LogEnd;
	return false;
    }

    return updatePrefix(client, duid, addr, iface, IAID, T1, T2, prefix, pref, valid, length, quiet);
}

bool TAddrMgr::updatePrefix(SPtr<TAddrClient> client, SPtr<TDUID> duid , SPtr<TIPv6Addr> clntAddr,
			    int iface, unsigned long IAID, unsigned long T1, unsigned long T2, 
			    SPtr<TIPv6Addr> prefix, unsigned long pref, unsigned long valid,
			    int length, bool quiet)
{
    if (!prefix) {
	Log(Error) << "Attempt to update null prefix failed." << LogEnd;
	return false;
    }
    if (!client) {
	Log(Error) << "Unable to update prefix, client not defined." << LogEnd;
	return false;
    }
    
    // for that client, find IA
    SPtr <TAddrIA> pd;
    client->firstPD();
    while ( pd = client->getPD() ) {
        if ( pd->getIAID() == IAID)
            break;
    }
    // have we found this PD?
    if (!pd) {
	Log(Error) << "Unable to find PD (iaid=" << IAID << ") for client " << duid->getPlain() << "." << LogEnd;
	return false;
    }
    pd->setTimestamp();
    pd->setT1(T1);
    pd->setT2(T2);

    SPtr <TAddrPrefix> ptrPrefix;
    pd->firstPrefix();
    while ( ptrPrefix = pd->getPrefix() ) {
        if (*ptrPrefix->get()==*prefix)
            break;
    }

    // address already exists
    if (!ptrPrefix) {
        Log(Warning) << "PD: Prefix " << prefix->getPlain() << " is not known. Unable to update." << LogEnd;
        return false;
    }

    ptrPrefix->setTimestamp();
    ptrPrefix->setPref(pref);
    ptrPrefix->setValid(pref);
    
    return true;
}

/*
 *  Frees prefix (also deletes IA and/or client, if this was last address)
 */
bool TAddrMgr::delPrefix(SPtr<TDUID> clntDuid,
			    unsigned long IAID, SPtr<TIPv6Addr> prefix,
			    bool quiet) {

    Log(Debug) << "PD: Deleting prefix " << prefix->getPlain() << ", DUID=" << clntDuid->getPlain() << ", iaid=" << IAID << LogEnd;
    // find this client
    SPtr <TAddrClient> ptrClient;
    this->firstClient();
    while ( ptrClient = this->getClient() ) {
        if ( (*ptrClient->getDUID()) == (*clntDuid) ) 
            break;
    }

    // have we found this client? 
    if (!ptrClient) {
        Log(Warning) << "PD: Client (DUID=" << clntDuid->getPlain() 
		     << ") not found in addrDB, cannot delete address and/or client." << LogEnd;
	return false;
    }

    // find this IA
    SPtr <TAddrIA> ptrPD;
    ptrClient->firstPD();
    while ( ptrPD = ptrClient->getPD() ) {
        if ( ptrPD->getIAID() == IAID)
            break;
    }

    // have we found this IA?
    if (!ptrPD) {
        Log(Warning) << "PD: iaid=" << IAID << " not assigned to client, cannot delete address and/or PD."
		     << LogEnd;
        return false;
    }

    SPtr <TAddrPrefix> ptrPrefix;
    ptrPD->firstPrefix();
    while ( ptrPrefix = ptrPD->getPrefix() ) {
        if (*ptrPrefix->get()==*prefix)
            break;
    }

    // address already exists
    if (!ptrPrefix) {
	Log(Warning) << "PD: Prefix " << *prefix << " not assigned, cannot delete." << LogEnd;
	return false;
    }

    ptrPD->delPrefix(prefix);

    /// @todo: Cache for prefixes this->addCachedAddr(clntDuid, clntAddr);
    if (!quiet)
      	Log(Debug) << "PD: Deleted prefix " << *prefix << " from addrDB." << LogEnd;
    
    if (!ptrPD->getPrefixCount()) {
	      if (!quiet)
	          Log(Debug) << "PD: Deleted PD (iaid=" << IAID << ") from addrDB." << LogEnd;
	      ptrClient->delPD(IAID);
    }

    if (!ptrClient->countIA() && !ptrClient->countTA() && !ptrClient->countPD() && DeleteEmptyClient) {
        if (!quiet)
	          Log(Debug) << "PD: Deleted client (DUID=" << clntDuid->getPlain()
                       << ") from addrDB." << LogEnd;
        this->delClient(clntDuid);
    }

    return true;
}


/** 
 * checks if a specific prefix is used
 * 
 * @param x 
 * 
 * @return true if prefix is free, false if it is used
 */
bool TAddrMgr::prefixIsFree(SPtr<TIPv6Addr> x)
{
    SPtr<TAddrClient> client;
    SPtr<TAddrIA> pd;
    SPtr<TAddrPrefix> prefix;

    firstClient();
    while (client = getClient()) {
	      client->firstPD();
	      while (pd = client->getPD()) {
	          pd->firstPrefix();
	          while (prefix = pd->getPrefix()) {
		            if (*prefix->get()==*x)
		                return false;
	          }
	      }
    }

    /* prefix not found, so it's free */
    return true;
}

// --------------------------------------------------------------------
// --- XML-related methods (libxml2) ----------------------------------
// --------------------------------------------------------------------
#ifdef MOD_LIBXML2
// loads entire file
xmlDocPtr TAddrMgr::xmlLoad(const char * filename) {
     xmlDocPtr doc;
     xmlNodePtr cur;

     doc = xmlParseFile(filename);
	
     if (doc == NULL ) {
 	fprintf(stderr,"Document not parsed successfully. \n");
 	return NULL;
     }

     cur = xmlDocGetRootElement(doc);
	
     if (cur == NULL) {
 	fprintf(stderr,"empty document\n");
 	xmlFreeDoc(doc);
 	return NULL;
     }

     if (xmlStrcmp(cur->name, (const xmlChar *) "AddrMgr")) {
 	fprintf(stderr,"document of the wrong type, root node != AddrMgr\n");
 	xmlFreeDoc(doc);
 	return NULL;
     }

     // DTD
     xmlDtdPtr dtd = xmlParseDTD(NULL, ADDRDB_DTD); /* parse the DTD */
     if (!dtd)
     {
 	Log(Error) << "DTD load failed." << LogEnd;
 	//return NULL;
     }

     return doc;
  //  return NULL;
}

SPtr<TAddrAddr> TAddrMgr::parseAddrAddr(xmlDocPtr doc, xmlNodePtr xmlAddr, int depth)
{
     // timestamp
     xmlChar * tsStr = xmlGetProp(xmlAddr,(const xmlChar*)"timestamp");
     unsigned long ts = atol((const char *)tsStr);

     // prefered-lifetime
     xmlChar * prefStr = xmlGetProp(xmlAddr,(const xmlChar*)"pref");
     unsigned long pref = atol((const char *)prefStr);

     // valid-lifetime
     xmlChar * validStr = xmlGetProp(xmlAddr,(const xmlChar*)"valid");
     unsigned long valid = atol((const char *)validStr);

     // address
     xmlChar * addr;
     addr = xmlNodeListGetString(doc, xmlAddr->xmlChildrenNode, 1);
     char addrPacked[16];

     inet_pton6((const char*)addr,addrPacked);

     SPtr<TAddrAddr> ptrAddr = new TAddrAddr(new TIPv6Addr(addrPacked),pref,valid);
     ptrAddr->setTimestamp(ts);

     xmlFree(addr);
     xmlFree(tsStr);
     xmlFree(prefStr);
     xmlFree(validStr);

     return ptrAddr;
//   return NULL;
}

SPtr<TAddrIA> TAddrMgr::libxml_parseAddrIA(xmlDocPtr doc, xmlNodePtr xmlIA, int depth)
{
     // DUID
     xmlChar * DUIDStr = xmlGetProp(xmlIA,(const xmlChar*)"DUID");
     SPtr <TDUID> ptrDUID = new TDUID((char*)DUIDStr);

     // unicast
     xmlChar * unicast = xmlGetProp(xmlIA,(const xmlChar*)"unicast");
     char packedUnicast[16];
     SPtr<TIPv6Addr> ptrUnicast;
     if (unicast) {
       inet_pton6((const char *)unicast,packedUnicast);
       ptrUnicast = new TIPv6Addr(packedUnicast);
     }

     // T1
     xmlChar * T1Str = xmlGetProp(xmlIA,(const xmlChar*)"T1");
     unsigned long T1 = atol((const char *)T1Str);

     // T2
     xmlChar * T2Str = xmlGetProp(xmlIA,(const xmlChar*)"T2");
     unsigned long T2 = atol((const char *)T2Str);
    
     // IAID
     xmlChar * IAIDStr = xmlGetProp(xmlIA,(const xmlChar*)"IAID");
     unsigned long IAID = atol((const char *)IAIDStr);

     // state
     xmlChar * stateStr = xmlGetProp(xmlIA,(const xmlChar*)"state");
     unsigned long state = atol((const char *)stateStr);

     // iface
     xmlChar * ifaceStr = xmlGetProp(xmlIA,(const xmlChar*)"iface");
     unsigned long iface = atol((const char *)ifaceStr);

     // now we've got all data, so create AddrIA
     SPtr<TAddrIA> ptrIA = new TAddrIA(iface, ptrUnicast, ptrDUID, T1, T2, IAID);

     xmlFree(DUIDStr);
     xmlFree(unicast);
     xmlFree(T1Str);
     xmlFree(T2Str);
     xmlFree(IAIDStr);
     xmlFree(stateStr);
     xmlFree(ifaceStr);
    
     // look for each address
     SPtr<TAddrAddr> ptrAddr;
     xmlNodePtr xmlAddr = xmlIA->xmlChildrenNode;
     while (xmlAddr) {
 	if (xmlAddr->type == XML_ELEMENT_NODE) {
 	    ptrAddr = parseAddrAddr(doc, xmlAddr, depth+2);
 	    ptrIA->addAddr(ptrAddr);
	    ptrAddr->setTentative(TENTATIVE_NO);
 	}
 	xmlAddr = xmlAddr->next;
     }

     return ptrIA;
//    return NULL;
}

SPtr<TAddrClient> TAddrMgr::parseAddrClient(xmlDocPtr doc, xmlNodePtr xmlClient, int depth)
{
     // DUID
     xmlChar *DUID;
     DUID = xmlGetProp(xmlClient,(const xmlChar*)"DUID");
     SPtr<TDUID> ptrDUID = new TDUID((char*)DUID);

     // create AddrClient
     SPtr<TAddrClient> ptrClient = new TAddrClient(ptrDUID);

     xmlFree(DUID);

     SPtr<TAddrIA> ptrIA;
     xmlNodePtr xmlIA = xmlClient->xmlChildrenNode;
     while (xmlIA) {
 	if (xmlIA->type == XML_ELEMENT_NODE) {
 	    ptrIA = libxml_parseAddrIA(doc, xmlIA, depth+2);
 	    ptrClient->addIA(ptrIA);
 	}
 	xmlIA = xmlIA->next;
     }
     return ptrClient;
//    return NULL;
}

void TAddrMgr::parseAddrMgr(xmlDocPtr doc,int depth)
{
     xmlNodePtr n;
     xmlNodePtr xmlClient;
     n = xmlDocGetRootElement(doc);

     xmlClient = n->xmlChildrenNode;

     SPtr<TAddrClient> ptrClient;

     while (xmlClient) {
 	if (xmlClient->type == XML_ELEMENT_NODE) {
 	    ptrClient = parseAddrClient(doc,xmlClient,depth+2);

 	    // append this client to clients list
 	    this->ClntsLst.append(ptrClient);
 	}
 	xmlClient = xmlClient->next;
     }
}
#else

// --------------------------------------------------------------------
// --- XML-related methods (built-in) ---------------------------------
// --------------------------------------------------------------------
/**
 * @brief loads AddrMgr database from a file
 *
 * loads AddrMgr database from a file. Opens specified
 * XML file and parsed outer \<AddrMgr>\</AddrMgr> tags.
 *
 * @param xmlFile filename that contains database
 *
 * @return database load status: true=success, false if there were problems
 */
bool TAddrMgr::xmlLoadBuiltIn(const char * xmlFile)
{
    SPtr<TAddrClient> clnt;
    bool AddrClient = false;

    char buf[256];

    FILE * f;
    if (!(f = fopen(xmlFile,"r"))) {
        Log(Warning) << "Unable to open " << xmlFile << "." << LogEnd;
        return false;
    }

    while (!feof(f)) {
        fgets(buf, 255, f);
        if (strstr(buf,"<AddrMgr>")) {
            AddrClient = true;
            continue;
        }
        if (AddrClient && strstr(buf,"<AddrClient")) {
            clnt = parseAddrClient(f);
	    ClntsLst.append(clnt);
	    Log(Debug) << "Client " << clnt->getDUID()->getPlain() << " loaded from disk successfuly." << LogEnd;
            continue;
        }

        if (strstr(buf,"</AddrMgr>")) {
            break;
        }
    }
    fclose(f);

    if (clnt) 
	return true; // client detected, then file loading was successful
    
    return false;
}

/**
 * @brief parses XML section that defines single client
 *
 * parses XML section that defines single client.
 * That is <AddrClient>...</AddrClient> section.
 *
 * @param f file handle
 *
 * @return pointer to a newly created TAddrClient object
 */
SPtr<TAddrClient> TAddrMgr::parseAddrClient(FILE *f)
{
    char buf[256];
    char * x = 0;
    bool AddrIA = false;
    bool AddrPD = false;
    bool AddrTA = false;
    int t1 = 0, t2 = 0, iaid = 0, pdid = 0, iface = 0;

    SPtr<TAddrClient> clnt = 0;
    SPtr<TDUID> duid = 0;
    SPtr<TAddrIA> ia = 0;
    SPtr<TAddrIA> ptrpd=0;
    SPtr<TAddrIA> ta = 0;

    while (!feof(f)) {
    	fgets(buf,255,f);
        AddrIA=false;

	if (strstr(buf,"<duid")) {
            x = strstr(buf,">")+1;
            x = strstr(x,"</duid>");
            if (x)
      	        *x = 0; // remove trailing xml tag
            duid = new TDUID(strstr(buf,">")+1);
            clnt = new TAddrClient(duid);
            /// @todo: support for more than one IA

            /// @todo: support for PD
            continue;
        }
	if(strstr(buf,"<AddrIA ")){
	    AddrIA=true;
	    t1 = 0; t2 = 0; iaid = 0; iface = 0;
	    if ((x=strstr(buf,"T1"))) {
		t1=atoi(x+4);
		// Log(Debug) << "Parsed AddrIA::T1=" << t1 << LogEnd;
	    }
	    if ((x=strstr(buf,"T2"))) {
		t2=atoi(x+4);
		// Log(Debug) << "Parsed AddrIA::T2=" << t2 << LogEnd;
	    }
	    if ((x=strstr(buf,"IAID"))) {
		iaid=atoi(x+6);
		// Log(Debug) << "Parsed AddrIA::IAID=" << iaid << LogEnd;
	    }
	    if ((x=strstr(buf,"iface"))) {
		iface=atoi(x+7);
		// Log(Debug) << "Parsed AddrIA::iface=" << iface << LogEnd;
	    }
	    if (ia = parseAddrIA(f,AddrIA,t1,t2,iaid,iface)) {
		clnt->addIA(ia);
		Log(Debug) << "Parsed IA, iaid=" << iaid << LogEnd;
            }
	}
        if(strstr(buf,"<AddrTA ")){
	    AddrTA=true;
            ta = parseAddrTA(f);
        }
	if(strstr(buf,"<AddrPD ")){
	    AddrPD=true;
	    t1 = 0; t2 = 0; pdid = 0; iface = 0;
	    if ((x=strstr(buf,"T1"))) {
		t1=atoi(x+4);
		// Log(Debug) << "Parsed AddrPD::T1=" << t1 << LogEnd;
	    }
	    if ((x=strstr(buf,"T2"))) {
		t2=atoi(x+4);
		// Log(Debug) << "Parsed AddrPD::T2=" << t2 << LogEnd;
	    }
	    if ((x=strstr(buf,"PDID"))) {
		pdid=atoi(x+6);
		// Log(Debug) << "Parsed AddrPD::PDID=" << pdid << LogEnd;
	    }
	    if ((x=strstr(buf,"iface"))) {
		iface=atoi(x+7);
		// Log(Debug) << "Parsed AddrPD::iface=" << iface << LogEnd;
	    }
	    if (ptrpd = parseAddrPD(f,AddrPD,t1,t2,pdid,iface)) {
		clnt->addPD(ptrpd);
		Log(Debug) << "Parsed PD, pdid=" << pdid << LogEnd;
            }
	}
	if (strstr(buf,"</AddrClient>"))
	    break;
    }

    /// @todo: add some extra checks here
    return clnt;
}


/** 
 * parses TA definition
 * just a dummy function for now. Temporary addresses are ignored completely
 * 
 * @param f file handle
 * 
 * @return will return parsed temporary IA someday. Returns 0 now
 */
SPtr<TAddrIA> TAddrMgr::parseAddrTA(FILE *f) {
    char buf[256];
    while(!feof(f)) {
         fgets(buf,255,f);
	 if (strstr(buf,"</AddrTA>")){
	          break;
	}
    }
    return 0;
}

/**
 * @brief parses part XML section that represents single PD
 *
 * (section between <AddrPD>...</AddrPD>)
 *
 * @param f file handle
 * @param AddrPD is this AddrPD token
 * @param t1 T1 value
 * @param t2 T2 value
 * @param iaid IAID
 * @param iface interface index
 *
 * @return pointer to newly created TAddrIA object
 */
SPtr<TAddrIA> TAddrMgr::parseAddrPD(FILE * f, bool AddrPD,int t1,int t2,int iaid,int iface) {
    // IA paramteres
    char buf[256];
    bool display = false;
    char * x = 0;
    SPtr<TAddrIA> ptrpd = 0;
    SPtr<TAddrAddr> addr;
    SPtr<TAddrPrefix> pr;
    SPtr<TDUID> duid;
    while (!feof(f) && AddrPD) {
	fgets(buf,255,f);
	if (t1!=0 && t2!=0 && iaid!=0 && iface!=0 && display==false) {
	    display=true;;
	    Log(Debug) << "Loaded PD from a file: t1=" << t1 << ", t2="<< t2 << ",iaid=" << iaid << ", iface=" << iface << LogEnd;
	    duid = 0; // don't use old DUID
	}
	if (strstr(buf,"duid")) {
	    //char * x;
	    x = strstr(buf,">")+1;
	    x = strstr(x,"</duid>");
	    if (x)
		*x = 0; // remove trailing xml tag
	    duid = new TDUID(strstr(buf,">")+1);
	    // Log(Debug) << "Parsed IA: duid=" << duid->getPlain() << LogEnd;
	    ptrpd = new TAddrIA(iface, TAddrIA::TYPE_PD, 0, duid, t1, t2, iaid);
	    ptrpd->setState(STATE_CONFIRMME);
	    continue;
	}
	if (strstr(buf,"<AddrPrefix")) {
	    pr = parseAddrPrefix(buf,AddrPD);
	    if (ptrpd && pr) {
		ptrpd->addPrefix(pr);
		pr->setTentative(TENTATIVE_NO);
		//Log(Debug) << "Parsed prefix " << pr->getPlain() << LogEnd;
	    }
	}
	if (strstr(buf,"</AddrPD>"))
	    break;
    }
    if (ptrpd)
  	ptrpd->setTentative();
    return ptrpd;
}

/**
 * @brief parses part XML section that represents single IA
 *
 * (section between <AddrIA>...</AddrIA>)
 *
 * @param f file handle
 * @param AddrIA
 * @param t1 parsed T1 timer value
 * @param t2 parsed T2 timer value
 * @param iaid parsed IAID 
 * @param iface parsed interface index (ifindex)
 *
 * @return pointer to newly created TAddrIA object
 */

SPtr<TAddrIA> TAddrMgr::parseAddrIA(FILE * f, bool AddrIA,int t1,int t2,int iaid,int iface)
{
    // IA paramteres
    char buf[256];
    char * x = 0;
    //int t1 = 0, t2 = 0, iaid = 0, iface = 0;
    SPtr<TAddrIA> ia = 0;
    SPtr<TAddrAddr> addr;
    SPtr<TDUID> duid;
    while (!feof(f)) {
	fgets(buf,255,f);
	if (t1!=0 && t2!=0 && iaid!=0 && iface!=0 && AddrIA==true) {
		  AddrIA=false;
		  Log(Debug) << "Loaded IA from a file: t1=" << t1 << ", t2="<< t2 << ",iaid=" << iaid << ", iface=" << iface << LogEnd;
		  duid = 0; // don't use old DUID
	}
	if (strstr(buf,"duid")) {
	          //char * x;
	          x = strstr(buf,">")+1;
	          x = strstr(x,"</duid>");
	          if (x)
      		      *x = 0; // remove trailing xml tag
	          duid = new TDUID(strstr(buf,">")+1);
	          // Log(Debug) << "Parsed IA: duid=" << duid->getPlain() << LogEnd;

	          ia = new TAddrIA(iface, TAddrIA::TYPE_IA, 0, duid, t1,t2, iaid);
	          ia->setState(STATE_CONFIRMME);
	          continue;
	      }
	      if (strstr(buf,"<AddrAddr")) {
	          addr = parseAddrAddr(buf,false);
	          if (ia && addr) {
		            ia->addAddr(addr);
		            addr->setTentative(TENTATIVE_NO);
	          }
	      }
	      if (strstr(buf,"</AddrIA>"))
	          break;
    }
    if (ia)
  	ia->setTentative();
    return ia;
}

/**
 * @brief parses single address
 *
 * parses single address that is defined in <AddrAddr> tag.
 *
 * @param buf null terminated buffer that contains string to parse
 *
 * @return pointer to the newly created TAddrAddr object
 */
SPtr<TAddrAddr> TAddrMgr::parseAddrAddr(char * buf, bool pd)
{
    // address parameters
    unsigned long timestamp, pref=DHCPV6_INFINITY, valid=DHCPV6_INFINITY;
    int prefix = CLIENT_DEFAULT_PREFIX_LENGTH;
    SPtr<TIPv6Addr> addr = 0;
    SPtr<TAddrAddr> addraddr;
    char * x;

    if (strstr(buf, "<AddrAddr") || strstr(buf, "<AddrPrefix")) {
	      timestamp=pref=valid=0;
	      addr = 0;
	      if ((x=strstr(buf,"timestamp"))) {
	          timestamp = atoi(x+11);
	      }
	      if ((x=strstr(buf,"pref="))) {
	          pref = atoi(x+6);
	      }
	      if ((x=strstr(buf,"valid"))) {
		  valid = atoi(x+7);
	      }
	      if(pd==false) {
		  if ((x=strstr(buf,"prefix="))) {
		      prefix = atoi(x+8);
		  }
	      }
	      else {
		  if ((x=strstr(buf,"length="))) {
		      prefix = atoi(x+8);
		  }
       	      }	
	      if ((x=strstr(buf,">"))) {
		  if(pd==false)
		      x = strstr(x, "</AddrAddr>");
		  else
		      x = strstr(x, "</AddrPrefix>");
	          if (x)
	      	      *x = 0;
	          addr = new TIPv6Addr(strstr(buf,">")+1, true);
	          Log(Debug) << "Parsed addr=" << addr->getPlain() << ", pref=" << pref << ", valid=" << valid << ",ts=" << timestamp << LogEnd;

	      }
	      if (addr && timestamp && pref && valid && pd==false) {
	          addraddr = new TAddrAddr(addr, pref, valid, prefix);
	          addraddr->setTimestamp(timestamp);
	      }
    }
    return addraddr;
}

SPtr<TAddrPrefix> TAddrMgr::parseAddrPrefix(char * buf, bool pd)
{
    // address parameters
    unsigned long timestamp, pref=DHCPV6_INFINITY, valid=DHCPV6_INFINITY;
    int prefix = CLIENT_DEFAULT_PREFIX_LENGTH,lenght;
    SPtr<TIPv6Addr> addr = 0;
    SPtr<TAddrPrefix> addraddr;
    char * x;

    if (strstr(buf, "<AddrAddr") || strstr(buf, "<AddrPrefix")) {
	      timestamp=pref=valid=0;
	      addr = 0;
	      if ((x=strstr(buf,"timestamp"))) {
	          timestamp = atoi(x+11);
	      }
	      if ((x=strstr(buf,"pref="))) {
	          pref = atoi(x+6);
	      }
	      if ((x=strstr(buf,"valid"))) {
	          	valid = atoi(x+7);
	      }
	      if(pd==true) {
	      		if ((x=strstr(buf,"length="))) {
	          		prefix = atoi(x+8);
	      		}
	      }
	      else {
			if ((x=strstr(buf,"prefix="))) {
	          		prefix = atoi(x+8);
	      		}
       	      }	
	      if ((x=strstr(buf,">"))) {
		  if(pd==true)
			x = strstr(x, "</AddrPrefix>");
		  else
			x = strstr(x, "</AddrAddr>");
	          if (x)
	      	      *x = 0;
	          addr = new TIPv6Addr(strstr(buf,">")+1, true);
	          Log(Debug) << "Parsed addr=" << addr->getPlain() << ", pref=" << pref << ", valid=" << valid << ",ts=" << timestamp << LogEnd;

	      }
	      if (addr && timestamp && pref && valid && pd==true) {
		lenght=prefix;
	          addraddr = new TAddrPrefix(addr, pref, valid, lenght);
	          addraddr->setTimestamp(timestamp);
	      }
    }
    return addraddr;
}





#endif

/**
 * @brief returns if shutdown is complete
 *
 * returns is AddrMgr completed its operations and is ready
 * to conclude shutdown.
 *
 * @return true - I'm done. false - keep going
 */
bool TAddrMgr::isDone() {
    return this->IsDone;
}

TAddrMgr::~TAddrMgr() {

}

// --------------------------------------------------------------------
// --- operators ------------------------------------------------------
// --------------------------------------------------------------------

ostream & operator<<(ostream & strum,TAddrMgr &x) {
    strum << "<AddrMgr>" << endl;
    strum << "  <timestamp>" << now() << "</timestamp>" << endl;
    x.print(strum);

    SPtr<TAddrClient> ptr;
    x.ClntsLst.first();

    while ( ptr = x.ClntsLst.get() ) {
        strum << *ptr;
    }

    strum << "</AddrMgr>" << endl;
    return strum;
}
