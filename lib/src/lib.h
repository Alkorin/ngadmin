
#ifndef DEF_LIB
#define DEF_LIB


#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>

#include <ngadmin.h>


#define PASSWORD_MAX	32


#define CLIENT_PORT		63321
#define SWITCH_PORT		63322

#define CODE_READ_REQ		1
#define CODE_READ_REP		2
#define CODE_WRITE_REQ		3
#define CODE_WRITE_REP		4

#define ERROR_READONLY		3
#define ERROR_INVALID_VALUE	5
#define ERROR_DENIED		7

#define ATTR_PRODUCT		0x0001
#define ATTR_UNK_0002		0x0002
#define ATTR_NAME		0x0003
#define ATTR_MAC		0x0004
#define ATTR_UNK_0005		0x0005
#define ATTR_IP			0x0006
#define ATTR_NETMASK		0x0007
#define ATTR_GATEWAY		0x0008
#define ATTR_NEW_PASSWORD	0x0009
#define ATTR_PASSWORD		0x000A
#define ATTR_DHCP		0x000B
#define ATTR_UNK_000C		0x000C
#define ATTR_FIRM_VER		0x000D
#define ATTR_UNK_000E		0x000E
#define ATTR_UNK_000F		0x000F
#define ATTR_FIRM_UPGRADE	0x0010
#define ATTR_RESTART		0x0013
#define ATTR_DEFAULTS		0x0400
#define ATTR_PORT_STATUS	0x0C00
#define ATTR_PORT_STATISTICS	0x1000
#define ATTR_STATS_RESET	0x1400
#define ATTR_CABLETEST_DO	0x1800
#define ATTR_CABLETEST_RESULT	0x1C00
#define ATTR_VLAN_TYPE		0x2000
#define ATTR_VLAN_PORT_CONF	0x2400
#define ATTR_VLAN_DOT_CONF	0x2800
#define ATTR_VLAN_DESTROY	0x2C00
#define ATTR_VLAN_PVID		0x3000
#define ATTR_QOS_TYPE		0x3400
#define ATTR_QOS_CONFIG		0x3800
#define ATTR_BITRATE_INPUT	0x4C00
#define ATTR_BITRATE_OUTPUT	0x5000
#define ATTR_STORM_ENABLE	0x5400
#define ATTR_STORM_BITRATE	0x5800
#define ATTR_MIRROR		0x5C00
#define ATTR_PORTS_COUNT	0x6000
#define ATTR_MAX_VLAN		0x6400
#define ATTR_IGMP_ENABLE_VLAN	0x6800
#define ATTR_IGMP_BLOCK_UNK	0x6C00
#define ATTR_IGMP_VALID_V3	0x7000
#define ATTR_TLV_BITMAP		0x7400
#define ATTR_END		0xFFFF



struct ngadmin {
 // network
 int sock;			// socket
 struct sockaddr_in local;	// local address & port
 struct in_addr brd;		// broadcast address
 char iface[IFNAMSIZ];		// interface
 struct timeval timeout;	// timeout
 struct ether_addr localmac;	// local MAC address
 bool keepbroad;		// keep broadcasting
 bool globalbroad;		// use global broadcast address (255.255.255.255)
 // 
 char password[PASSWORD_MAX];	// password to use to login on switches
 struct swi_attr *swi_tab;	// array of detected switches
 int swi_count;			// number of detected switches
 struct swi_attr *current;	// administred switch
 int seq;			// sequence number for packets
};



#endif

