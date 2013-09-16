
#ifndef DEF_LIB
#define DEF_LIB


#include <arpa/inet.h>

#include <net/if.h>
#include <netinet/ether.h>

#include <ngadmin.h>



#define PASSWORD_MAX		32


struct ngadmin {
	/* network */
	int sock;			/* socket */
	struct sockaddr_in local;	/* local address & port */
	struct in_addr brd;		/* broadcast address */
	char iface[IFNAMSIZ];		/* interface */
	struct timeval timeout;		/* timeout */
	struct ether_addr localmac;	/* local MAC address */
	bool keepbroad;			/* keep broadcasting */
	bool globalbroad;		/* use global broadcast address (255.255.255.255) */
	/* switch properties */
	bool encrypt_pass;		/* switch uses password "encryption" */
	char password[PASSWORD_MAX];	/* password to use to login on switches */
	struct swi_attr *swi_tab;	/* array of detected switches */
	int swi_count;			/* number of detected switches */
	struct swi_attr *current;	/* administred switch */
	int seq;			/* sequence number for packets */
};


#endif

