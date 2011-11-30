
#ifndef DEF_PROTOCOL
#define DEF_PROTOCOL


#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include <ngadmin.h>
#include "list.h"
#include "lib.h"





struct ng_header {
 char unk1;			// always 1, maybe version
 char code;			// request code: read request, read reply, write request, write reply
 unsigned short error;		// error code, 0 when no error
 unsigned short attr; 		// attribute code which caused error, 0 when no error
 char unk2[2]; 			// always 0, unknown
 char client_mac[ETH_ALEN];	// client MAC address
 char switch_mac[ETH_ALEN];	// switch MAC address
 unsigned int seqnum;		// sequence number
 char proto_id[4]; 		// always "NSDP", maybe short for "Netgear Switch Description Protocol"
 char unk3[4]; 			// always 0, unknown
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



extern const struct ether_addr nullMac;




// 
int trim (char *txt, int start);

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
struct attr* newAttr (unsigned short attr, unsigned short size, void *data);

// 
struct attr* newByteAttr (unsigned short attr, unsigned char value);

// 
struct attr* newShortAttr (unsigned short attr, short value);

// 
struct attr* newIntAttr (unsigned short attr, int value);

// 
struct attr* newAddrAttr (unsigned short attr, struct in_addr value);

// 
void freeAttr (struct attr *at);

// 
void extractPacketAttributes (struct ng_packet *np, unsigned short *error, unsigned short *attr_error, List *attr);

// 
void extractSwitchAttributes (struct swi_attr *sa, const List *l);




#endif

