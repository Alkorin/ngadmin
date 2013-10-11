
#ifndef DEF_NETWORK
#define DEF_NETWORK


#include <nsdp/list.h>
#include <nsdp/net.h>
#include "lib.h"


int startNetwork (struct ngadmin *nga);


int setBroadcastType (struct ngadmin *nga, bool value);


int stopNetwork (struct ngadmin *nga);


int forceInterface (struct ngadmin *nga);


void prepareSend (struct ngadmin *nga, struct nsdp_cmd *nc, unsigned char code);


void prepareRecv (struct ngadmin *nga, struct nsdp_cmd *nc, unsigned char code);


int readRequest (struct ngadmin *nga, List *attr);


int writeRequest (struct ngadmin *nga, List *attr);


void extractSwitchAttributes (struct swi_attr *sa, const List *l);


#endif

