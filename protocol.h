
#ifndef DEF_PROTOCOL
#define DEF_PROTOCOL


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include "list.h"
#include "command.h"




#define CLIENT_PORT		63321
#define SWITCH_PORT		63322

#define CODE_READ_REQ		1
#define CODE_READ_REP		2
#define CODE_WRITE_REQ		3
#define CODE_WRITE_REP		4

#define ATTR_PRODUCT		0x0001
#define ATTR_UNK2		0x0002
#define ATTR_NAME		0x0003
#define ATTR_MAC		0x0004
#define ATTR_UNK5		0x0005
#define ATTR_IP			0x0006
#define ATTR_NETMASK		0x0007
#define ATTR_GATEWAY		0x0008
#define ATTR_NEW_PASSWORD	0x0009
#define ATTR_PASSWORD		0x000A
#define ATTR_DHCP		0x000B
#define ATTR_UNK12		0x000C
#define ATTR_FIRM_VER		0x000D
#define ATTR_UNK14		0x000E
#define ATTR_UNK15		0x000F
#define ATTR_RESTART		0x0013
#define ATTR_DEFAULTS		0x0400
#define ATTR_PORT_STATUS	0x0C00
#define ATTR_VLAN_TYPE		0x2000
#define ATTR_VLAN_CONFIG	0x2400
#define ATTR_QOS_TYPE		0x3400
#define ATTR_QOS_CONFIG		0x3800
#define ATTR_MIRROR		0x5C00
#define ATTR_PORTS_NUMBER	0x6000
#define ATTR_END		0xFFFF

#define SPEED_DOWN		0
#define SPEED_10		1
#define SPEED_100		4
#define SPEED_1000		5

#define VLAN_PORT_BASIC		1
#define VLAN_PORT_ADV		2
#define VLAN_DOT_BASIC		3
#define VLAN_DOT_ADV		4

#define QOS_PORT		1
#define QOS_DOT			2

#define PRIO_HIGH		1
#define PRIO_MED		2
#define PRIO_NORM		3
#define PRIO_LOW		4



struct ng_header {
 char unk1; // always 1
 char code;
 char unk2[6]; // always 0
 char client_mac[6];
 char switch_mac[6];
 unsigned int seqnum;
 char proto_id[4]; // always "NSDP"
 char unk3[4]; // always 0
 char data[0];
} __attribute__((packed)) ;

struct attr_header {
 unsigned short attr;
 unsigned short size;
 char data[0];
} __attribute__((packed)) ;


struct ng_packet {
 union {
  char *buffer;
  struct ng_header *nh;
 };
 int maxlen;
 struct attr_header *ah;
};


struct attr {
 unsigned short attr;
 unsigned short size;
 void *data;
};



extern const unsigned short helloRequest[];

extern const struct ether_addr nullMac;



// 
int min (int a, int b);

// 
void initNgHeader (struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum);

// 
bool validateNgHeader (const struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum);

// 
void initNgPacket (struct ng_packet *np);

// 
void addPacketAttr (struct ng_packet *np, unsigned short attr, unsigned short size, void* data);

// 
void addPacketEmptyAttr (struct ng_packet *np, unsigned short attr);

// 
void addPacketByteAttr (struct ng_packet *np, unsigned short attr, char val);

// 
void addPacketShortAttr (struct ng_packet *np, unsigned short attr, short val);

// 
int getPacketTotalSize (const struct ng_packet *np);

// 
struct attr* newEmptyAttr (unsigned short attr);

// 
void freeAttr (struct attr *at);

// 
List* extractPacketAttributes (struct ng_packet *np);





#endif

