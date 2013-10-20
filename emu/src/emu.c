
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <arpa/inet.h>
#include <getopt.h>

#include <sys/ioctl.h>
#include <net/if.h>

#include <ngadmin.h> /* FIXME */
#include <nsdp/protocol.h>
#include <nsdp/attr.h>
#include <nsdp/net.h>
#include <nsdp/misc.h>


#define MAX_STR_SIZE	64


struct port_info {
	/* read-only properties */
	unsigned char state;
	unsigned long long recv;
	unsigned long long sent;
	unsigned long long crc;
	/* configurable properties */
	unsigned short pvid;
	int bitrate_in;
	int bitrate_out;
	unsigned char prio;
};


struct swi_info {
	/* intrinsic properties */
	unsigned char mac[ETH_ALEN];
	char product[MAX_STR_SIZE];
	char firmware[MAX_STR_SIZE];
	unsigned char ports_count;
	bool encpass;
	/* configurable properties */
	char name[MAX_STR_SIZE];
	char password[MAX_STR_SIZE];
	in_addr_t ip;
	in_addr_t netmask;
	in_addr_t gw;
	bool dhcp;
	/* ports specifics */
	struct port_info *ports;
};


static struct swi_info swi;
static int sock;


static int process_read_attr (struct nsdp_cmd *nc, List *attr, struct attr *at)
{
	unsigned char p, *byte;
	unsigned short *word;
	unsigned int *dword;
	struct attr_port_status *apu;
	struct attr_port_stat *api;
	struct attr_bitrate *ab;
	
	
	(void)nc;
	
	switch (at->attr) {
	}
	
	at->size = 0;
	free(at->data);
	at->data = NULL;
	
	switch (at->attr) {
	
	case ATTR_PRODUCT:
		byte = malloc(MAX_STR_SIZE);
		memcpy(byte, swi.product, MAX_STR_SIZE);
		at->data = byte;
		at->size = MAX_STR_SIZE;
		break;
	
	case ATTR_NAME:
		byte = malloc(MAX_STR_SIZE);
		memcpy(byte, swi.name, MAX_STR_SIZE);
		at->data = byte;
		at->size = MAX_STR_SIZE;
		break;
	
	case ATTR_FIRM_VER:
		byte = malloc(MAX_STR_SIZE);
		memcpy(byte, swi.firmware, MAX_STR_SIZE);
		at->data = byte;
		at->size = MAX_STR_SIZE;
		break;
	
	case ATTR_MAC:
		byte = malloc(ETH_ALEN);
		memcpy(byte, swi.mac, ETH_ALEN);
		at->data = byte;
		at->size = ETH_ALEN;
		break;
	
	case ATTR_PORTS_COUNT:
		byte = malloc(1);
		*byte = swi.ports_count;
		at->data = byte;
		at->size = 1;
		break;
	
	case ATTR_IP:
		byte = malloc(sizeof(in_addr_t));
		*(in_addr_t*)byte = swi.ip;
		at->data = byte;
		at->size = sizeof(in_addr_t);
		break;
	
	case ATTR_NETMASK:
		byte = malloc(sizeof(in_addr_t));
		*(in_addr_t*)byte = swi.netmask;
		at->data = byte;
		at->size = sizeof(in_addr_t);
		break;
	
	case ATTR_GATEWAY:
		byte = malloc(sizeof(in_addr_t));
		*(in_addr_t*)byte = swi.gw;
		at->data = byte;
		at->size = sizeof(in_addr_t);
		break;
	
	case ATTR_DHCP:
		/* Note: DHCP attribute is special, it is 2 two bytes long
		 * when sent by the switch but only 1 byte long when sent
		 * by the client
		 */
		 word = malloc(2);
		 *word = swi.dhcp;
		 at->data = word;
		 at->size = 2;
		break;
	
	case ATTR_ENCPASS:
		if (!swi.encpass)
			break;
		dword = malloc(4);
		*dword = 1;
		at->data = dword;
		at->size = 4;
		break;
	
	case ATTR_PORT_STATUS:
		for (p = 0; p < swi.ports_count; p++) {
			apu = malloc(sizeof(*apu));
			apu->port = p + 1;
			apu->status = swi.ports[p].state;
			apu->unk = 0;
			
			pushFrontList(attr, newAttr(ATTR_PORT_STATUS, sizeof(*apu), apu));
		}
		return 1;
	
	case ATTR_PORT_STATISTICS:
		for (p = 0; p < swi.ports_count; p++) {
			api = malloc(sizeof(*api));
			memset(api, 0, sizeof(*api));
			api->port = p + 1;
			api->recv = swi.ports[p].recv;
			api->sent = swi.ports[p].sent;
			
			pushFrontList(attr, newAttr(ATTR_PORT_STATISTICS, sizeof(*api), api));
		}
		return 1;
	
	case ATTR_BITRATE_INPUT:
		for (p = 0; p < swi.ports_count; p++) {
			ab = malloc(sizeof(*ab));
			ab->port = p + 1;
			ab->bitrate = swi.ports[p].bitrate_in;
			
			pushFrontList(attr, newAttr(ATTR_BITRATE_INPUT, sizeof(*ab), ab));
		}
		return 1;
	
	case ATTR_BITRATE_OUTPUT:
		for (p = 0; p < swi.ports_count; p++) {
			ab = malloc(sizeof(*ab));
			ab->port = p + 1;
			ab->bitrate = swi.ports[p].bitrate_out;
			
			pushFrontList(attr, newAttr(ATTR_BITRATE_OUTPUT, sizeof(*ab), ab));
		}
		return 1;
	
	}
	
	return 0;
}


static int process_write_attr (struct nsdp_cmd *nc, List *attr, struct attr *at)
{
	unsigned char p, *byte;
	char *text;
	struct attr_bitrate *ab;
	
	
	(void)attr;
	
	if (at->size == 0)
		return -EMSGSIZE;
	
	switch (at->attr) {
	
	case ATTR_NEW_PASSWORD:
		if (swi.encpass)
			passwordEndecode(at->data, at->size);
		text = at->data;
		trim(text, MAX_STR_SIZE);
		strncpy(swi.password, text, MAX_STR_SIZE);
		break;
	
	case ATTR_STATS_RESET:
		for (p = 0; p < swi.ports_count; p++) {
			swi.ports[p].sent = 0;
			swi.ports[p].recv = 0;
			swi.ports[p].crc = 0;
		}
		break;
	
	case ATTR_DHCP:
		/* Note: DHCP attribute is special, it is 2 two bytes long
		 * when sent by the switch but only 1 byte long when sent
		 * by the client
		 */
		if (at->size != 1) {
			nc->error = 4;
			break;
		}
		byte = at->data;
		swi.dhcp = (*byte == 1);
		break;
	
	case ATTR_IP:
	case ATTR_NETMASK:
	case ATTR_GATEWAY:
		/* a real switch would accept these modifications, but here we
		 * are not going to mess up the host network settings, so we
		 * refuse these requests
		 */
		nc->error = ERROR_READONLY;
		nc->attr_error = at->attr;
		break;
	
	case ATTR_BITRATE_INPUT:
		ab = at->data;
		if (ab->port > swi.ports_count)
			return -EOVERFLOW;
		swi.ports[ab->port - 1].bitrate_in = ab->bitrate;
		break;
	
	case ATTR_BITRATE_OUTPUT:
		ab = at->data;
		if (ab->port > swi.ports_count)
			return -EOVERFLOW;
		swi.ports[ab->port - 1].bitrate_out = ab->bitrate;
		break;
	}
	
	
	return 1;
}


static int check_password (struct nsdp_cmd *nc, List *attr)
{
	ListNode *ln = attr->first;
	struct attr *at = NULL;
	char *text;
	
	
	/* in a write request, the password attribute must be present
	 * and the first element of the list
	 * official win app never sends password inside a read request, but
	 * ngadmin does that because in that case the password is not echoed
	 * back by the switch, so we have to support that, though password in
	 * such requests are not mandatory
	 */
	if (ln == NULL || (at = ln->data)->attr != ATTR_PASSWORD) {
		if (nc->code == CODE_WRITE_REQ)
			nc->error = ERROR_DENIED;
		goto end;
	}
	
	if (at->size == 0) {
		nc->error = ERROR_DENIED;
		goto end;
	}
	
	/* normally, we would expect password encryption to be handled
	 * in read requests as well as in write requests
	 * but it seems that real Netgear switches that support
	 * password encryption do NOT accept encrypted passwords in
	 * read requests
	 * this seems more to be a bug in their firmwares, however, as
	 * the scope of this program is to simulate a switch, we adopt
	 * the same buggy behaviour
	 */
	if (nc->code == CODE_WRITE_REQ && swi.encpass)
		passwordEndecode(at->data, at->size);
	text = at->data;
	text[at->size] = '\0';
	trim(text, at->size);
	if (strcmp(text, swi.password) != 0)
		nc->error = ERROR_DENIED;
	
	
end:
	if (nc->error == 0) {
		/* correct password */
		if (ln != NULL && at->attr == ATTR_PASSWORD)
			destroyElement(attr, ln, (void(*)(void*))freeAttr);
		return 0;
	} else {
		/* invalid password, empty list */
		nc->attr_error = ATTR_PASSWORD;
		clearList(attr, (void(*)(void*))freeAttr);
		return -EACCES;
	}
}


static void process_packet (struct nsdp_cmd *nc, List *attr)
{
	struct ListNode *ln, *pr;
	struct attr *at;
	int err;
	int (*process_attr)(struct nsdp_cmd *, List *, struct attr *);
	
	
	check_password(nc, attr);
	if (nc->code == CODE_READ_REQ) {
		nc->code = CODE_READ_REP;
		process_attr = process_read_attr;
	} else if (nc->code == CODE_WRITE_REQ) {
		nc->code = CODE_WRITE_REP;
		process_attr = process_write_attr;
	} else {
		/* invalid operation code */
		return;
	}
	
	
	for (ln = attr->first; ln != NULL; ) {
		at = ln->data;
		
		err = process_attr(nc, attr, at);
		
		if (err == 1) {
		 	/* destroy current attribute */
			pr = ln;
			ln = ln->next;
			destroyElement(attr, pr, (void(*)(void*))freeAttr);
		} else if (err == 0) {
			/* keep current attribute */
			ln = ln->next;
		} else {
			/* error, abort */
			return;
		}
	}
	
	memcpy(&nc->switch_mac, swi.mac, ETH_ALEN);
	nc->remote_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	
	sendNsdpPacket(sock, nc, attr);
}


static int netdev_info (const char *dev)
{
	struct ifreq ifr;
	int err;
	
	
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, dev, IFNAMSIZ - 1);
	
	/* get interface MAC */
	err = ioctl(sock, SIOCGIFHWADDR, &ifr, sizeof(struct ifreq));
	if (err < 0) {
		perror("ioctl(SIOCGIFHWADDR)");
		return err;
	}
	memcpy(swi.mac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	
	/* get interface IP */
	err = ioctl(sock, SIOCGIFADDR, &ifr, sizeof(struct ifreq));
	if (err < 0) {
		perror("ioctl(SIOCGIFADDR)");
		return err;
	}
	swi.ip = (*(struct sockaddr_in*)&ifr.ifr_addr).sin_addr.s_addr;
	
	/* get interface netmask */
	err = ioctl(sock, SIOCGIFNETMASK, &ifr, sizeof(struct ifreq));
	if (err < 0) {
		perror("ioctl(SIOCGIFNETMASK)");
		return err;
	}
	swi.netmask = (*(struct sockaddr_in*)&ifr.ifr_netmask).sin_addr.s_addr;
	
	/* TODO: get netmask */
	swi.gw = htonl(ntohl(swi.ip & swi.netmask) | 1);
	
	
	return 0;
}


static void handler (int sig)
{
	(void)sig;
	printf("interrupt\n");
}


int main (int argc, char **argv)
{
	static const struct option options[] = {
		{"help", no_argument, NULL, 'h'},
		{"interface", required_argument, NULL, 'i'},
		{0, 0, 0, 0}
	};
	int err = 0, len;
	unsigned char p;
	const char *iface = "eth0";
	struct sockaddr_in local;
	struct sigaction sa;
	struct nsdp_cmd nc;
	List *attr;
	
	
	opterr = 0;
	
	while ((len = getopt_long(argc, argv, "hi:", options, NULL)) != -1) {
		switch (len) {
		
		case 'h':
			printf("usage: %s [-h] [-i <interface>]\n", argv[0]);
			goto end;
		
		case 'i':
			iface = optarg;
			break;
		
		case '?':
			printf("unknown option: \"%s\"\n", argv[optind - 1]);
			goto end;
		}
	}
	
	
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock < 0) {
		perror("socket");
		err = 1;
		goto end;
	}
	
	memset(&local, 0, sizeof(struct sockaddr_in));
	local.sin_family = AF_INET;
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	local.sin_port = htons(SWITCH_PORT);
	
	if (bind(sock, (struct sockaddr*)&local, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		err = 1;
		goto end;
	}
	
	/* allow broadcasting */
	len = 1;
	len = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &len, sizeof(len));
	if (len < 0) {
		perror("setsockopt(SO_BROADCAST)");
		err = 1;
		goto end;
	}
	
	/* initialize switch infos */
	memset(&swi, 0, sizeof(struct swi_info));
	strncpy(swi.product, "NgEmu_0.1", MAX_STR_SIZE);
	strncpy(swi.name, "Netgear Switch Emulator", MAX_STR_SIZE);
	strncpy(swi.firmware, "0.1", MAX_STR_SIZE);
	strncpy(swi.password, "password", MAX_STR_SIZE);
	swi.encpass = true;
	swi.ports_count = 8;
	swi.dhcp = false;
	swi.ports = malloc(swi.ports_count * sizeof(struct port_info));
	memset(swi.ports, 0, swi.ports_count * sizeof(struct port_info));
	for (p = 0; p < swi.ports_count; p++) {
		swi.ports[p].state = SPEED_1000;
		swi.ports[p].pvid = 1;
		swi.ports[p].bitrate_in = BITRATE_NOLIMIT;
		swi.ports[p].bitrate_out = BITRATE_NOLIMIT;
		swi.ports[p].prio = PRIO_NORM;
	}
	if (netdev_info(iface) < 0)
		goto end;
	
	
	attr = createEmptyList();
	
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	
	while (1) {
		memset(&nc, 0, sizeof(struct nsdp_cmd));
		nc.remote_addr.sin_family = AF_INET;
		nc.remote_addr.sin_port = htons(CLIENT_PORT);
		
		len = recvNsdpPacket(sock, &nc, attr, NULL);
		if (len < 0)
			break;
		
		/* ignore packets not for us */
		for (len = 0; nc.switch_mac.ether_addr_octet[len] == 0 && len < ETH_ALEN; len++);
		if (len < ETH_ALEN && memcmp(swi.mac, &nc.switch_mac, ETH_ALEN) != 0)
			continue;
		
		process_packet(&nc, attr);
		clearList(attr, (void(*)(void*))freeAttr);
	}
	
	destroyList(attr, (void(*)(void*))freeAttr);
	free(swi.ports);
	
end:
	close(sock);
	
	return err;
}


