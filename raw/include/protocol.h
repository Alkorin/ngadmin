
#ifndef DEF_PROTOCOL
#define DEF_PROTOCOL


#include <stdlib.h>
#include <arpa/inet.h>

#include <netinet/ether.h>

#include "list.h"


#define CLIENT_PORT		63321
#define SWITCH_PORT		63322

#define CODE_READ_REQ		1
#define CODE_READ_REP		2
#define CODE_WRITE_REQ		3
#define CODE_WRITE_REP		4

#define ERROR_READONLY		3
#define ERROR_INVALID_VALUE	5
#define ERROR_DENIED		7


struct ng_header {
	char version;			/* always 1, maybe version */
	char code;			/* request code: read request, read reply, write request, write reply */
	unsigned char error;		/* error code, 0 when no error */
	unsigned char unk1;		/* always 0, unknown */
	unsigned short attr;		/* attribute code which caused error, 0 when no error */
	char unk2[2];			/* always 0, unknown */
	char client_mac[ETH_ALEN];	/* client MAC address */
	char switch_mac[ETH_ALEN];	/* switch MAC address */
	unsigned int seqnum;		/* sequence number */
	char proto_id[4];		/* always "NSDP", maybe short for "Netgear Switch Description Protocol" */
	char unk3[4];			/* always 0, unknown */
	char data[0];
} __attribute__((packed));


struct attr_header {
	unsigned short attr;
	unsigned short size;
	char data[0];
} __attribute__((packed));


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


extern const char passwordKey[];


void passwordEndecode (char *buf, unsigned int len);


int trim (char *txt, int start);


static inline int min (int a, int b)
{
	return a < b ? a : b;
}


void initNgHeader (struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum);


bool validateNgHeader (const struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum);


static inline void initNgPacket (struct ng_packet *np)
{
	np->ah = (struct attr_header*)np->nh->data;
}


void addPacketAttr (struct ng_packet *np, struct attr *at);


static inline int getPacketTotalSize (const struct ng_packet *np)
{
	return ((char*)np->ah) - np->buffer;
}


struct attr* newAttr (unsigned short attr, unsigned short size, void *data);


static inline struct attr* newEmptyAttr (unsigned short attr)
{
	return newAttr(attr, 0, NULL);
}


static inline struct attr* newByteAttr (unsigned short attr, unsigned char value)
{
	char *v = malloc(sizeof(char));
	
	*v = value;
	
	return newAttr(attr, sizeof(char), v);
}


static inline struct attr* newShortAttr (unsigned short attr, short value)
{
	short *v = malloc(sizeof(short));
	
	*v = value;
	
	return newAttr(attr, sizeof(short), v);
}


static inline struct attr* newIntAttr (unsigned short attr, int value)
{
	int *v = malloc(sizeof(int));
	
	*v = value;
	
	return newAttr(attr, sizeof(int), v);
}


static inline struct attr* newAddrAttr (unsigned short attr, struct in_addr value)
{
	struct in_addr *v = malloc(sizeof(struct in_addr));
	
	*v = value;
	
	return newAttr(attr, sizeof(struct in_addr), v);
}


void freeAttr (struct attr *at);


int addPacketAttributes (struct ng_packet *np, const List* attr, unsigned char ports);


int extractPacketAttributes (struct ng_packet *np, List *attr, unsigned char ports);


void filterAttributes (List *attr, ...);


#endif

