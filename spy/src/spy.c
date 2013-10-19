
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <ctype.h>
#include <signal.h>
#include <arpa/inet.h>

#include <ngadmin.h> /* FIXME */
#include <nsdp/protocol.h>
#include <nsdp/attr.h>
#include <nsdp/net.h>
#include <nsdp/misc.h>


static void print_password (const char *pass, unsigned int len)
{
	unsigned int i;
	char *s;
	
	
	for (i = 0; i < len; i++) {
		if (!isprint(pass[i]))
			break;
	}
	
	if (i == len) {
		/* all characters are printable, cleartext password assumed */
		printf("(clear) \"%s\"", pass);
		return;
	}
	
	s = malloc(len + 1);
	if (s == NULL)
		return;
	memcpy(s, pass, len);
	s[len] = '\0';
	
	passwordEndecode(s, len);
	
	for (i = 0; i < len; i++) {
		if (!isprint(s[i]))
			break;
	}
	
	if (i == len) {
		/* all characters are printable, encrypted password assumed */
		printf("(encrypted) \"%s\"", s);
		free(s);
		return;
	}
	
	free(s);
	
	/* unknown encoding, fallback to hex display */
	printf("(hex) ");
	for (i = 0; i < len; i++)
		printf("%02x ", (unsigned char)pass[i]);
}


static const char* port_status_str (unsigned char status)
{
	switch (status) {
	
	case SPEED_DOWN:
		return "down";
	
	case SPEED_10:
		return "10M";
	
	case SPEED_100:
		return "100M";
	
	case SPEED_1000:
		return "1000M";
	
	default:
		return "unknown";
	}
}


static const char* vlan_type_code (unsigned char type)
{
	switch (type) {
	
	case VLAN_DISABLED:
		return "disabled";
	
	case VLAN_PORT_BASIC:
		return "basic port based";
	
	case VLAN_PORT_ADV:
		return "advanced port based";
	
	case VLAN_DOT_BASIC:
		return "basic 802.1Q";
	
	case VLAN_DOT_ADV:
		return "advanced 802.1Q";
	
	default:
		return "unknown";
	}
}


static const char* vlan_code_str (unsigned char code)
{
	switch (code) {
	
	case VLAN_NO:
		return "no";
	
	case VLAN_UNTAGGED:
		return "untagged";
	
	case VLAN_TAGGED:
		return "tagged";
	
	default:
		return "unknown";
	}
}


static const char* qos_type_str (unsigned char type)
{
	switch (type) {
	
	case QOS_PORT:
		return "port";
	
	case QOS_DOT:
		return "802.1p";
	
	default:
		return "unknown";
	}
}


static const char* qos_prio_str (unsigned char prio)
{
	switch (prio) {
	
	case PRIO_HIGH:
		return "high";
	
	case PRIO_MED:
		return "medium";
	
	case PRIO_NORM:
		return "normal";
	
	case PRIO_LOW:
		return "low";
	
	default:
		return "unknown";
	}
}


static const char* bitrate_str (unsigned int bitrate)
{
	switch (bitrate) {
	
	case BITRATE_NOLIMIT:
		return "unlimited";
	
	case BITRATE_512K:
		return "512K";
	
	case BITRATE_1M:
		return "1M";
	
	case BITRATE_2M:
		return "2M";
	
	case BITRATE_4M:
		return "4M";
	
	case BITRATE_8M:
		return "8M";
	
	case BITRATE_16M:
		return "16M";
	
	case BITRATE_32M:
		return "32M";
	
	case BITRATE_64M:
		return "64M";
	
	case BITRATE_128M:
		return "128M";
	
	case BITRATE_256M:
		return "256M";
	
	case BITRATE_512M:
		return "512M";
	
	default:
		return "unknown";
	}
}


static void print_attr (const struct attr *at)
{
	unsigned char p, ports, *byte = at->data;
	unsigned short *word = at->data;
	char *s = at->data;
	struct ether_addr *eth = at->data;
	struct in_addr *ip = at->data;
	struct attr_port_status *apsu = at->data;
	struct attr_port_stat *apsi = at->data;
	struct attr_cabletest_do *acd = at->data;
	struct attr_cabletest_result *acr = at->data;
	struct attr_vlan_conf *avc = at->data;
	struct attr_pvid *ap = at->data;
	struct attr_qos *aq = at->data;
	struct attr_bitrate *ab = at->data;
	struct attr_mirror *am = at->data;
	struct attr_igmp_vlan *aiv = at->data;
	
	
	printf("code = %04X, length = %d\n", at->attr, at->size);
	
	if (at->size == 0) {
		printf("\tempty\n\n");
		return;
	}
	
	switch (at->attr) {
	
	case ATTR_PRODUCT:
		trim(s, at->size);
		printf("\tproduct = %s\n", s);
		break;
	
	case ATTR_NAME:
		trim(s, at->size);
		printf("\tname = %s\n", s);
		break;
	
	case ATTR_MAC:
		printf("\tMAC = %s\n", ether_ntoa(eth));
		break;
	
	case ATTR_IP:
		printf("\tIP = %s\n", inet_ntoa(*ip));
		break;
	
	case ATTR_NETMASK:
		printf("\tnetmask = %s\n", inet_ntoa(*ip));
		break;
	
	case ATTR_GATEWAY:
		printf("\tgateway = %s\n", inet_ntoa(*ip));
		break;
	
	case ATTR_NEW_PASSWORD:
		printf("\tnew password = ");
		print_password(s, at->size);
		printf("\n");
		break;
	
	case ATTR_PASSWORD:
		printf("\tpassword = ");
		print_password(s, at->size);
		printf("\n");
		break;
	
	case ATTR_DHCP:
		printf("\tDHCP = %s\n", (at->size == 1 ? *byte : *word) ? "yes" : "no");
		break;
	
	case ATTR_FIRM_VER:
		trim(s, at->size);
		printf("\tfirmware = %s\n", s);
		break;
	
	case ATTR_FIRM_UPGRADE:
		printf("\tfirmware upgrade requested\n");
		break;
	
	case ATTR_RESTART:
		printf("\trestart requested\n");
		break;
	
	case ATTR_ENCPASS:
		printf("\tpassword must be encrypted\n");
		break;
	
	case ATTR_DEFAULTS:
		printf("\treset parameters requested\n");
		break;
	
	case ATTR_PORT_STATUS:
		printf("\tport status\n");
		printf("\tport = %u\n", apsu->port);
		printf("\tstate = %s\n", port_status_str(apsu->status));
		break;
	
	case ATTR_PORT_STATISTICS:
		printf("\tport statistics\n");
		printf("\tport = %u\n", apsi->port);
		printf("\treceived = %llu\n", apsi->recv);
		printf("\tsend = %llu\n", apsi->sent);
		printf("\tCRC = %llu\n", apsi->crc);
		break;
	
	case ATTR_STATS_RESET:
		printf("\tport statistics reset requested\n");
		break;
	
	
	case ATTR_CABLETEST_DO:
		printf("\tstart cabletest\n");
		printf("\tport = %u\n", acd->port);
		printf("\taction = %u\n", acd->action);
		break;
	
	case ATTR_CABLETEST_RESULT:
		printf("\tcabletest result\n");
		printf("\tport = %u\n", acr->port);
		if (at->size > 1) {
			printf("\tv1 = %u\n", acr->v1);
			printf("\tv2 = %u\n", acr->v2);
		}
		break;
	
	case ATTR_VLAN_TYPE:
		printf("\tVLAN type = %s\n", vlan_type_code(*byte));
		break;
	
	case ATTR_VLAN_PORT_CONF:
		printf("\tport based VLAN configuration\n");
		printf("\tVLAN = %u\n", avc->vlan);
		ports = at->size - sizeof(struct attr_vlan_conf);
		for (p = 0; p < ports; p++)
			printf("\tport %d = %s\n", p + 1, vlan_code_str(avc->ports[p]));
		break;
	
	case ATTR_VLAN_DOT_CONF:
		printf("\t802.1Q based VLAN configuration\n");
		printf("\tVLAN = %u\n", avc->vlan);
		ports = at->size - sizeof(struct attr_vlan_conf);
		for (p = 0; p < ports; p++)
			printf("\tport %d = %s\n", p + 1, vlan_code_str(avc->ports[p]));
		break;
	
	case ATTR_VLAN_DESTROY:
		printf("\tdestroy VLAN = %u\n", *word);
		break;
	
	case ATTR_VLAN_PVID:
		printf("\tVLAN PVID\n");
		printf("\tport = %u\n", ap->port);
		printf("\tVLAN = %u\n", ap->vlan);
		break;
	
	case ATTR_QOS_TYPE:
		printf("\tQoS type = %s\n", qos_type_str(*byte));
		break;
	
	case ATTR_QOS_CONFIG:
		printf("\tQoS configuration\n");
		printf("\tport = %u\n", aq->port);
		printf("\tpriority = %s\n", qos_prio_str(aq->prio));
		break;
	
	case ATTR_BITRATE_INPUT:
		printf("\tinput bitrate\n");
		printf("\tport = %u\n", ab->port);
		printf("\tbitrate = %s\n", bitrate_str(ab->bitrate));
		break;
	
	case ATTR_BITRATE_OUTPUT:
		printf("\toutput bitrate\n");
		printf("\tport = %u\n", ab->port);
		printf("\tbitrate = %s\n", bitrate_str(ab->bitrate));
		break;
	
	case ATTR_STORM_ENABLE:
		printf("\tstorm filtering = %s\n", *byte ? "yes" : "no");
		break;
	
	case ATTR_STORM_BITRATE:
		printf("\tstorm filtering bitrate\n");
		printf("\tport = %u\n", ab->port);
		printf("\tbitrate = %s\n", bitrate_str(ab->bitrate));
		break;
	
	case ATTR_MIRROR:
		printf("\tport mirroring\n");
		if (am->outport == 0) {
			printf("\tdisabled\n");
			break;
		}
		
		printf("\toutput port = %u\n", am->outport);
		ports = at->size - sizeof(struct attr_mirror);
		for (p = 0; p < ports; p++)
			printf("\tport %u = %s\n", p + 1, am->ports[p] ? "yes" : "no");
		break;
	
	case ATTR_PORTS_COUNT:
		printf("\tports count = %u\n", *byte);
		break;
	
	case ATTR_IGMP_ENABLE_VLAN:
		printf("\tIGMP filtering\n");
		printf("\tenable = %s\n", aiv->enable ? "yes" : "no");
		printf("\tVLAN = %u\n", aiv->vlan);
		break;
	
	case ATTR_IGMP_BLOCK_UNK:
		printf("\tblock unknown IGMP = %s\n", *byte ? "yes" : "no");
		break;
	
	case ATTR_IGMP_VALID_V3:
		printf("\tvalidate IGMPv3 = %s\n", *byte ? "yes" : "no");
		break;
	
	default:
		printf("\tunknown\n");
	}
	
	printf("\n");
}


static const char* code_str (unsigned char code)
{
	switch (code) {
	
	case CODE_READ_REQ:
		return "read request";
	
	case CODE_READ_REP:
		return "read reply";
	
	case CODE_WRITE_REQ:
		return "write request";
	
	case CODE_WRITE_REP:
		return "write reply";
	
	default:
		return "unknown";
	}
}


static const char* error_str (unsigned char err)
{
	switch (err) {
	
	case 0:
		return "none";
	
	case ERROR_READONLY:
		return "read only";
	
	case ERROR_INVALID_VALUE:
		return "invalid value";
	
	case ERROR_DENIED:
		return "access denied";
	
	default:
		return "unknown";
	}
}


static void print_packet (const List *attr, const struct nsdp_cmd *nc)
{
	const ListNode *ln;
	const struct attr *at;
	
	
	printf("---------------------------------\n");
	printf("code = %s (%u)\n", code_str(nc->code), nc->code);
	printf("error = %s (%u)\n", error_str(nc->error), nc->error);
	if (nc->attr_error != 0)
		printf("erroneous attribute = %04X\n", nc->attr_error);
	printf("source address = %s:%u\n", inet_ntoa(nc->remote_addr.sin_addr), ntohs(nc->remote_addr.sin_port));
	printf("client MAC = %s\n", ether_ntoa(&nc->client_mac));
	printf("switch MAC = %s\n", ether_ntoa(&nc->switch_mac));
	printf("sequence number = %u\n\n", nc->seqnum);
	printf("received %d attribute(s)\n\n", attr->count);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		print_attr(at);
	}
	
	printf("---------------------------------\n\n");
}


static void handler (int sig)
{
	(void)sig;
	printf("interrupt\n");
}


int main (void)
{
	int err = 0, sw_sock = -1, cl_sock = -1;
	List *attr;
	struct nsdp_cmd nc;
	struct sockaddr_in sw_local, cl_local;
	struct pollfd fds[2];
	struct sigaction sa;
	
	
	sw_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sw_sock < 0) {
		perror("socket");
		err = 1;
		goto end;
	};
	
	cl_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (cl_sock < 0) {
		perror("socket");
		err = 1;
		goto end;
	};
	
	
	memset(&sw_local, 0, sizeof(struct sockaddr_in));
	sw_local.sin_family = AF_INET;
	sw_local.sin_addr.s_addr = htonl(INADDR_ANY);
	sw_local.sin_port = htons(SWITCH_PORT);
	
	cl_local = sw_local;
	cl_local.sin_port = htons(CLIENT_PORT);
	
	if (bind(sw_sock, (struct sockaddr*)&sw_local, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		err = 2;
		goto end;
	}
	
	if (bind(cl_sock, (struct sockaddr*)&cl_local, sizeof(struct sockaddr_in)) < 0) {
		perror("bind");
		err = 2;
		goto end;
	}
	
	fds[0].fd = sw_sock;
	fds[0].events = POLLIN;
	fds[0].revents = 0;
	fds[1].fd = cl_sock;
	fds[1].events = POLLIN;
	fds[1].revents = 0;
	
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = handler;
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	
	attr = createEmptyList();
	
	while (1) {
		err = poll(fds, 2, -1);
		if (err < 0) {
			perror("poll");
			break;
		} else if (err == 0) {
			continue;
		}
		
		memset(&nc, 0, sizeof(struct nsdp_cmd));
		nc.remote_addr.sin_family = AF_INET;
		
		if (fds[0].revents & POLLIN) {
			nc.remote_addr.sin_port = htons(CLIENT_PORT);
			err = recvNsdpPacket(sw_sock, &nc, attr, NULL);
		} else {
			nc.remote_addr.sin_port = htons(SWITCH_PORT);
			err = recvNsdpPacket(cl_sock, &nc, attr, NULL);
		}
		
		if (err < 0)
			continue;
		
		print_packet(attr, &nc);
		
		clearList(attr, (void(*)(void*))freeAttr);
	}
	
	destroyList(attr, (void(*)(void*))freeAttr);
	
end:
	close(sw_sock);
	close(cl_sock);
	
	return err;
}

