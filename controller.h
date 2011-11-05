
#ifndef DEF_CONTROLLER
#define DEF_CONTROLLER


#include <stdbool.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

#include "list.h"
#include "protocol.h"
#include "network.h"



#define ERR_OK		0
#define ERR_NET		-1
#define ERR_NOTLOG	-2
#define ERR_BADPASS	-3
#define ERR_BADID	-4
#define ERR_INVARG	-5
#define ERR_TIMEOUT	-6

#define PRODUCT_SIZE	64
#define NAME_SIZE	64
#define FIRMWARE_SIZE	64



struct net_conf {
 struct in_addr ip, netmask, gw;
 bool dhcp;
};


struct swi_attr {
 char product[PRODUCT_SIZE];
 char name[NAME_SIZE];
 char firmware[FIRMWARE_SIZE];
 unsigned char ports;
 struct ether_addr mac;
 struct net_conf nc;
};





void startController (void);

void stopController (void);



int discoverSwitches (void);

const List* getSwitchList (void);

const struct swi_attr* getCurrentSwitch (void);

int login (int i);

int logout (void);



void setPassword (const char *pass);

int changePassword (const char *pass);



int getFirmwareVersion (char *buf, int size);

int getMirrorPorts (unsigned char *ports, int size);

int setMirrorPorts (const unsigned char *ports, int size);



int getPortsNumber (void);

int getPortsStatus (unsigned char *ports, int size);

int getPortsStatistics (unsigned long long stats[][6], int size);

int resetPortsStatistics (void);



int getQosStatus (void);

int setQosMode (char mode);

int setQosPorts (const unsigned char* ports, int size, char level);



int getNetConf (struct net_conf *nc);

int setNetConf (const struct net_conf *nc);



int getName (char *name, int size);

int setName (const char *name);



#endif

