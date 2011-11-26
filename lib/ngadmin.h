
#ifndef DEF_NGADMIN
#define DEF_NGADMIN


#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/ether.h>


#ifdef BUILD_LIB
#define EXPORT	__attribute__((visibility("default")))
#else
#define EXPORT	
#endif


#define ERR_OK		0	// no error
#define ERR_NET		-1	// network
#define ERR_NOTLOG	-2	// not logged on a switch
#define ERR_BADPASS	-3	// bad password
#define ERR_BADID	-4	// bad switch id
#define ERR_INVARG	-5	// invalid argument
#define ERR_TIMEOUT	-6	// timeout

#define PRODUCT_SIZE	64
#define NAME_SIZE	64
#define FIRMWARE_SIZE	64




#define SPEED_DOWN		0
#define SPEED_10		1
#define SPEED_100		4
#define SPEED_1000		5

#define VLAN_DISABLED		0
#define VLAN_PORT_BASIC		1
#define VLAN_PORT_ADV		2
#define VLAN_DOT_BASIC		3
#define VLAN_DOT_ADV		4

#define VLAN_NO			0
#define VLAN_UNTAGGED		1
#define VLAN_TAGGED		2

#define QOS_PORT		1
#define QOS_DOT			2

#define PRIO_UNSPEC		-1
#define PRIO_HIGH		1
#define PRIO_MED		2
#define PRIO_NORM		3
#define PRIO_LOW		4

#define BITRATE_UNSPEC		-1
#define BITRATE_NOLIMIT		0
#define BITRATE_512K		1
#define BITRATE_1M		2
#define BITRATE_2M		3
#define BITRATE_4M		4
#define BITRATE_8M		5
#define BITRATE_16M		6
#define BITRATE_32M		7
#define BITRATE_64M		8
#define BITRATE_128M		9
#define BITRATE_256M		10
#define BITRATE_512M		11




// opaque type to clients
struct ngadmin;


struct net_conf {
 struct in_addr ip;		// IP
 struct in_addr netmask;	// netmask
 struct in_addr gw;		// gateway IP
 bool dhcp;			// DHCP enabled
};


struct swi_attr {
 char product[PRODUCT_SIZE];	// product name (eg. GS108EV1)
 char name[NAME_SIZE];		// custom name
 char firmware[FIRMWARE_SIZE];	// firmware version string
 unsigned char ports;		// number of ports
 struct ether_addr mac;		// MAC address
 struct net_conf nc;		// network configuration
};


struct port_stats {
 unsigned long long recv;
 unsigned long long sent;
 unsigned long long crc;
};


struct igmp_conf {
 bool enable;
 unsigned short vlan;
 bool validate;
 bool block;
};


struct cabletest {
 char port;
 int v1;
 int v2;
};



// initialize NgAdmin lib
struct ngadmin* ngadmin_init (const char *iface) EXPORT;

// 
int ngadmin_close (struct ngadmin *nga) EXPORT;


// 
int ngadmin_forceInterface (struct ngadmin *nga) EXPORT;


// 
int ngadmin_setKeepBroadcasting (struct ngadmin *nga, bool value) EXPORT;


// specify password to use for authenticating on switches
int ngadmin_setPassword (struct ngadmin *nga, const char *pass) EXPORT;


// 
int ngadmin_setTimeout (struct ngadmin *nga, const struct timeval *tv) EXPORT;


// 
int ngadmin_scan (struct ngadmin *nga) EXPORT;


// 
const struct swi_attr* ngadmin_getSwitchTab (struct ngadmin *nga, int *nb) EXPORT;


// 
const struct swi_attr* ngadmin_getCurrentSwitch (struct ngadmin *nga) EXPORT;


// 
int ngadmin_login (struct ngadmin *nga, int id) EXPORT;


// 
int ngadmin_getPortsStatus (struct ngadmin *nga, unsigned char *ports) EXPORT;


// 
int ngadmin_setName (struct ngadmin *nga, const char *name) EXPORT;


// 
int ngadmin_getPortsStatistics (struct ngadmin *nga, struct port_stats *ps) EXPORT;


// 
int ngadmin_resetPortsStatistics (struct ngadmin *nga) EXPORT;


// 
int ngadmin_changePassword (struct ngadmin *nga, const char* pass) EXPORT;


// 
int ngadmin_getStormFilterState (struct ngadmin *nga, int *s) EXPORT;


// 
int ngadmin_setStormFilterState (struct ngadmin *nga, int s) EXPORT;


// 
int ngadmin_getStormFilterValues (struct ngadmin *nga, int *ports) EXPORT;


// 
int ngadmin_setStormFilterValues (struct ngadmin *nga, const int *ports) EXPORT;


// 
int ngadmin_getBitrateLimits (struct ngadmin *nga, int *ports) EXPORT;


// 
int ngadmin_setBitrateLimits (struct ngadmin *nga, const int *ports) EXPORT;


// 
int ngadmin_getQOSMode (struct ngadmin *nga, int *s) EXPORT;


// 
int ngadmin_setQOSMode (struct ngadmin *nga, int s) EXPORT;


// 
int ngadmin_getQOSValues (struct ngadmin *nga, char *ports) EXPORT;


// 
int ngadmin_setQOSValues (struct ngadmin *nga, const char *ports) EXPORT;


// 
int ngadmin_restart (struct ngadmin *nga) EXPORT;


// 
int ngadmin_defaults (struct ngadmin *nga) EXPORT;


// 
int ngadmin_getMirror (struct ngadmin *nga, char *ports) EXPORT;


// 
int ngadmin_setMirror (struct ngadmin *nga, const char *ports) EXPORT;


// 
int ngadmin_getIGMPConf (struct ngadmin *nga, struct igmp_conf *ic) EXPORT;


// 
int ngadmin_setIGMPConf (struct ngadmin *nga, const struct igmp_conf *ic) EXPORT;


// 
int ngadmin_cabletest (struct ngadmin *nga, struct cabletest *ct, int nb) EXPORT;


// 
int ngadmin_setNetConf (struct ngadmin *nga, const struct net_conf *nc) EXPORT;


// 
int ngadmin_getVLANType (struct ngadmin *nga, int *t) EXPORT;


// 
int ngadmin_getVLANDotConf (struct ngadmin *nga, char *buf, int *len) EXPORT;


// 
int ngadmin_getPVID (struct ngadmin *nga, unsigned short *ports) EXPORT;



#endif

