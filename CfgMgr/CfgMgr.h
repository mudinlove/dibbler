/*                                                                           
 * Dibbler - a portable DHCPv6                                               
 *                                                                           
 * authors: Tomasz Mrugalski <thomson@klub.com.pl>                           
 *          Marek Senderski <msend@o2.pl>                                    
 *                                                                           
 * released under GNU GPL v2 only licence                                
 *
 */

#ifndef CFGMGR_H
#define CFGMGR_H
#include <string>
#include "SmartPtr.h"
#include "DUID.h"
#include "IfaceMgr.h"

/* shared by server and relay */
#define RELAY_MIN_IFINDEX 1024

/* Defined DUID types */

enum EDUIDType{
    DUID_TYPE_NOT_DEFINED = 0,
    DUID_TYPE_LLT = 1,
    DUID_TYPE_EN  = 2,
    DUID_TYPE_LL  = 3  /* not supported */
};

class TCfgMgr
{
 public:
    enum DNSUpdateProtocol {
	DNSUPDATE_TCP, /* TCP only */
	DNSUPDATE_UDP, /* UDP only */
	DNSUPDATE_ANY  /* try UDP first, if response truncated, switch to TCP */
    };

    TCfgMgr();
    virtual ~TCfgMgr();

    bool compareConfigs(const std::string& cfgFile, const std::string& oldCfgFile);
    void copyFile(const std::string& cfgFile, const std::string& oldCfgFile);
    SPtr<TDUID> getDUID();
    void setWorkdir(std::string workdir);
    int getLogLevel();
    std::string getWorkDir();
    std::string getLogName();
    void setDDNSProtocol(DNSUpdateProtocol proto);
    DNSUpdateProtocol getDDNSProtocol() { return DdnsProto; }
    void setDDNSTimeout(unsigned int timeout) { _DDNSTimeout = timeout; }
    unsigned int getDDNSTimeout() { return _DDNSTimeout; }

 protected:
    SPtr<TDUID> DUID;
    bool setDUID(const std::string duidFile, TIfaceMgr &ifaceMgr);
    bool loadDUID(const std::string filename);
    bool generateDUID(const std::string duidFile,char * mac,int macLen, int macType);
    std::string Workdir;
    std::string LogName;
    int LogLevel;
    bool IsDone;
    EDUIDType DUIDType;
    int DUIDEnterpriseNumber;
    SPtr<TDUID> DUIDEnterpriseID;
    DNSUpdateProtocol DdnsProto;
    unsigned int _DDNSTimeout;
 private:
    
};
#endif
