
#ifndef DEF_NETWORK
#define DEF_NETWORK


#include "list.h"
#include "lib.h"


int startNetwork (struct ngadmin *nga);


int stopNetwork (struct ngadmin *nga);


int forceInterface (struct ngadmin *nga);


int updateTimeout (struct ngadmin *nga);


int sendNgPacket (struct ngadmin *nga, char code, const List *attr);


int recvNgPacket (struct ngadmin *nga, char code, unsigned char *error, unsigned short *attr_error, List *attr);


int readRequest (struct ngadmin *nga, List *attr);


int writeRequest (struct ngadmin *nga, List *attr);


void extractSwitchAttributes (struct swi_attr *sa, const List *l);


#endif

