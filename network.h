
#ifndef DEF_NETWORK
#define DEF_NETWORK


#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>

#include "list.h"
#include "protocol.h"



int startNetwork (const char *iface);

int stopNetwork (void);



int setTimeout (double sec);

int getTimeout (double *sec);



int sendNgPacket (char code, const struct ether_addr *switch_mac, unsigned int seqnum, const List *attr);

List* recvNgPacket (char code, const struct ether_addr *switch_mac, unsigned int seqnum);



#endif

