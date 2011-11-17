
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
 char unk1; // always 1
 char code;
 char error;
 char unk2; // always 0
 unsigned short attr; // attribute code which caused error
 char unk3[2]; // always 0
 char client_mac[6];
 char switch_mac[6];
 unsigned int seqnum;
 char proto_id[4]; // always "NSDP"
 char unk4[4]; // always 0
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
struct attr* newAttr (unsigned short attr, unsigned short size, void *data);

// 
struct attr* newByteAttr (unsigned short attr, unsigned char value);

// 
struct attr* newIntAttr (unsigned short attr, int value);

// 
void freeAttr (struct attr *at);

// 
List* extractPacketAttributes (struct ng_packet *np, char *error, unsigned short *attr_error);

// 
void extractSwitchAttributes (struct swi_attr *sa, const List *l);




#endif

