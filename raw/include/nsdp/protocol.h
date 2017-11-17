
#ifndef DEF_PROTOCOL
#define DEF_PROTOCOL


#include <netinet/ether.h>


#define CLIENT_PORT		63321
#define SWITCH_PORT		63322

#define NSDP_VERSION		1
#define NSDP_PROTOID		"NSDP"

#define CODE_READ_REQ		1
#define CODE_READ_REP		2
#define CODE_WRITE_REQ		3
#define CODE_WRITE_REP		4

#define ERROR_NONE		0
#define ERROR_READONLY		3
#define ERROR_WRITEONLY		4
#define ERROR_INVALID_VALUE	5
#define ERROR_DENIED		7


#define ATTR_PRODUCT		0x0001
#define ATTR_UNK_0002		0x0002
#define ATTR_NAME		0x0003
#define ATTR_MAC		0x0004
#define ATTR_UNK_0005		0x0005
#define ATTR_IP			0x0006
#define ATTR_NETMASK		0x0007
#define ATTR_GATEWAY		0x0008
#define ATTR_NEW_PASSWORD	0x0009
#define ATTR_PASSWORD		0x000A
#define ATTR_DHCP		0x000B
#define ATTR_UNK_000C		0x000C
#define ATTR_FIRM_VER		0x000D
#define ATTR_UNK_000E		0x000E
#define ATTR_UNK_000F		0x000F
#define ATTR_FIRM_UPGRADE	0x0010
#define ATTR_RESTART		0x0013
#define ATTR_ENCPASS		0x0014
#define ATTR_DEFAULTS		0x0400
#define ATTR_PORT_STATUS	0x0C00
#define ATTR_PORT_STATISTICS	0x1000
#define ATTR_STATS_RESET	0x1400
#define ATTR_CABLETEST_DO	0x1800
#define ATTR_CABLETEST_RESULT	0x1C00
#define ATTR_VLAN_TYPE		0x2000
#define ATTR_VLAN_PORT_CONF	0x2400
#define ATTR_VLAN_DOT_CONF	0x2800
#define ATTR_VLAN_DESTROY	0x2C00
#define ATTR_VLAN_PVID		0x3000
#define ATTR_QOS_TYPE		0x3400
#define ATTR_QOS_CONFIG		0x3800
#define ATTR_BITRATE_INPUT	0x4C00
#define ATTR_BITRATE_OUTPUT	0x5000
#define ATTR_STORM_ENABLE	0x5400
#define ATTR_STORM_BITRATE	0x5800
#define ATTR_MIRROR		0x5C00
#define ATTR_PORTS_COUNT	0x6000
#define ATTR_MAX_VLAN		0x6400
#define ATTR_IGMP_ENABLE_VLAN	0x6800
#define ATTR_IGMP_BLOCK_UNK	0x6C00
#define ATTR_IGMP_VALID_V3	0x7000
#define ATTR_TLV_BITMAP		0x7400
#define ATTR_LOOP_DETECT	0x9000
#define ATTR_END		0xFFFF



struct nsdp_header {
	unsigned char version;			/* always 1, maybe version */
	unsigned char code;			/* request code: read request, read reply, write request, write reply */
	unsigned char error;			/* error code, 0 when no error */
	unsigned char unk1;			/* always 0, unknown */
	unsigned short attr;			/* attribute code which caused error, 0 when no error */
	unsigned char unk2[2];			/* always 0, unknown */
	unsigned char client_mac[ETH_ALEN];	/* client MAC address */
	unsigned char switch_mac[ETH_ALEN];	/* switch MAC address */
	unsigned int seqnum;			/* sequence number */
	unsigned char proto_id[4];		/* always "NSDP", maybe short for "Netgear Switch Description Protocol" */
	unsigned char unk3[4];			/* always 0, unknown */
	unsigned char data[0];			/* attributes data */
} __attribute__((packed));


struct attr_header {
	unsigned short attr;		/* attribute code */
	unsigned short size;		/* attribute data size */
	unsigned char data[0];			/* attribute data */
} __attribute__((packed));



struct attr_port_status {
	unsigned char port;		/* port number */
	unsigned char status;		/* port status (speed index) */
	unsigned char unk;		/* unknown */
} __attribute__((packed));


struct attr_port_stat {
	unsigned char port;		/* port number */
	unsigned long long recv;	/* received bytes */
	unsigned long long sent;	/* sent bytes */
	unsigned long long unk1;	/* unknown */
	unsigned long long unk2;	/* unknown */
	unsigned long long unk3;	/* unknown */
	unsigned long long crc;		/* CRC errors */
} __attribute__((packed));


struct attr_bitrate {
	unsigned char port;		/* port number */
	int bitrate;			/* bitrate index */
} __attribute__((packed));


struct attr_qos {
	unsigned char port;		/* port number */
	unsigned char prio;		/* prio index */
} __attribute__((packed));


struct attr_pvid {
	unsigned char port;		/* port number */
	unsigned short vlan;		/* VLAN */
} __attribute__((packed));


struct attr_igmp_vlan {
	unsigned short enable;		/* IGMP filtering enabled */
	unsigned short vlan;		/* VLAN where IGMP packets are filtered */
} __attribute__((packed));


struct attr_cabletest_do {
	unsigned char port;		/* port number */
	unsigned char action;		/* action index */
} __attribute__((packed));


struct attr_cabletest_result {
	unsigned char port;		/* port number */
	unsigned int test_result;		/* Cable test result code */
	unsigned int fault_distance;		/* fault distance or cable length in meters */
} __attribute__((packed));


/* Note: this structure is not sent "as-is" on the wire.
 * A translation is done between the wire format (which uses somewhat not
 * trivial bitmap) and this simpler format. See attr.c for more details.
 */
struct attr_vlan_conf {
	unsigned short vlan;		/* VLAN number */
	unsigned char ports[0];		/* array, maps each port association with the VLAN */
};


/* Note: this structure is not sent "as-is" on the wire.
 * A translation is done between the wire format (which uses somewhat not
 * trivial bitmap) and this simpler format. See attr.c for more details.
 */
struct attr_mirror {
	unsigned char outport;		/* port number on which traffic is sent */
	unsigned char ports[0];		/* array, maps each port source mirror traffic */
};


#endif

