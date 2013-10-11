
#ifndef DEF_ENCODING
#define DEF_ENCODING


#include <stdbool.h>

#include <netinet/ether.h>

#include <protocol.h>
#include <list.h>


struct nsdp_packet {
	union {
		unsigned char *buffer;
		struct nsdp_header *nh;
	};
	int maxlen;
	struct attr_header *ah;
};



void initNsdpHeader (struct nsdp_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum);


bool validateNsdpHeader (const struct nsdp_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum);


static inline void initNsdpPacket (struct nsdp_packet *np)
{
	np->ah = (struct attr_header*)np->nh->data;
}


static inline int getPacketTotalSize (const struct nsdp_packet *np)
{
	return ((unsigned char*)np->ah) - np->buffer;
}


int addPacketAttributes (struct nsdp_packet *np, const List* attr, unsigned char ports);


int extractPacketAttributes (struct nsdp_packet *np, List *attr, unsigned char ports);


#endif

