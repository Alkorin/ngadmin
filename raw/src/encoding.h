
#ifndef DEF_ENCODING
#define DEF_ENCODING


#include <stdbool.h>

#include <netinet/ether.h>

#include <nsdp/protocol.h>
#include <nsdp/net.h>
#include <nsdp/list.h>


struct nsdp_packet {
	union {
		unsigned char *buffer;
		struct nsdp_header *nh;
	};
	int maxlen;
	struct attr_header *ah;
};



void initNsdpHeader (struct nsdp_header *nh, const struct nsdp_cmd *nc);


bool extractNsdpHeader (const struct nsdp_header *nh, struct nsdp_cmd *nc);


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

