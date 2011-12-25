
#ifndef DEF_NETWORK
#define DEF_NETWORK


#include <string.h>
#include <unistd.h>
//#include <poll.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
//#include <sys/time.h>

#include "list.h"
#include "lib.h"
#include "protocol.h"




// 
int startNetwork (struct ngadmin *nga);

// 
int stopNetwork (struct ngadmin *nga);

// 
int forceInterface (struct ngadmin *nga);

// 
int updateTimeout (struct ngadmin *nga);

// 
int sendNgPacket (struct ngadmin *nga, char code, const List *attr);

// 
int recvNgPacket (struct ngadmin *nga, char code, unsigned char *error, unsigned short *attr_error, List *attr);

// 
int readRequest (struct ngadmin *nga, List *attr);

// 
int writeRequest (struct ngadmin *nga, List *attr);



#endif

