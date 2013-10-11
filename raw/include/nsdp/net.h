
#ifndef DEF_NET
#define DEF_NET


#include <time.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include <nsdp/list.h>


struct nsdp_cmd {
	struct ether_addr client_mac;
	struct ether_addr switch_mac;
	struct sockaddr_in remote_addr;
	unsigned int seqnum;
	unsigned int ports;
	unsigned char code;
	unsigned char error;
	unsigned short attr_error;
};



int sendNsdpPacket (int sock, const struct nsdp_cmd *nc, const List *attr);


int recvNsdpPacket (int sock, struct nsdp_cmd *nc, List *attr, const struct timeval *timeout);


#endif

