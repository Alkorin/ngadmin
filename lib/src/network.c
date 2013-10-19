
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>

#include <nsdp/attr.h>
#include <nsdp/misc.h>
#include <nsdp/net.h>
#include <nsdp/protocol.h>

#include "network.h"


int startNetwork (struct ngadmin *nga)
{
	struct ifreq ifr;
	int ret;
	
	
	/* create socket */
	nga->sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (nga->sock < 0) {
		perror("socket");
		return nga->sock;
	}
	
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, nga->iface, IFNAMSIZ - 1);

	/* get the interface MAC address */
	ret = ioctl(nga->sock, SIOCGIFHWADDR, &ifr);
	if (ret < 0) {
		perror("ioctl(SIOCGIFHWADDR)");
		close(nga->sock);
		return ret;
	}
	memcpy(&nga->localmac, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
	
	/* bind */
	memset(&nga->local, 0, sizeof(struct sockaddr_in));
	nga->local.sin_family = AF_INET;
	nga->local.sin_port = htons(CLIENT_PORT);
	
	ret = bind(nga->sock, (struct sockaddr*)&nga->local, sizeof(struct sockaddr_in));
	if (ret < 0) {
		perror("bind");
		close(nga->sock);
		return ret;
	}
	
	/* allow broadcasting */
	ret = 1;
	ret = setsockopt(nga->sock, SOL_SOCKET, SO_BROADCAST, &ret, sizeof(ret));
	if (ret < 0) {
		perror("setsockopt(SO_BROADCAST)");
		return ret;
	}
	
	/* prevent unicast packets from being routed by setting the TTL to 1 */
	ret = 1;
	ret = setsockopt(nga->sock, IPPROTO_IP, IP_TTL, &ret, sizeof(ret));
	if (ret < 0) {
		perror("setsockopt(IP_TTL)");
		return ret;
	}
	
	
	return 0;
}


int setBroadcastType (struct ngadmin *nga, bool value)
{
	int ret;
	struct ifreq ifr;
	
	
	nga->globalbroad = value;
	if (value) {
		nga->brd.s_addr = (in_addr_t)0;
		return 0;
	}
	
	memset(&ifr, 0, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, nga->iface, IFNAMSIZ - 1);
	
	/* get the interface broadcast address */
	ret = ioctl(nga->sock, SIOCGIFBRDADDR, &ifr);
	if (ret < 0) {
		perror("ioctl(SIOCGIFBRDADDR)");
		nga->brd.s_addr = (in_addr_t)0;
		nga->globalbroad = true;
		return ret;
	}
	
	nga->brd = (*(struct sockaddr_in*)&ifr.ifr_addr).sin_addr;
	
	return 0;
}


int stopNetwork (struct ngadmin *nga)
{
	return close(nga->sock);
}


int forceInterface (struct ngadmin *nga)
{
	int ret;
	
	
	/* as described bellow, when you have multiple interfaces, this
	 * forces the packet to go to a particular interface
	 */
	ret = setsockopt(nga->sock, SOL_SOCKET, SO_BINDTODEVICE, nga->iface, strlen(nga->iface) + 1);
	if (ret < 0) {
		perror("setsockopt(SO_BINDTODEVICE)");
		return ret;
	}
	
	/* if the switch's IP is not in your network range, for instance
	 * because you do not have DHCP  enabled or you started the switch
	 * after your DHCP server, this allows to bypass the routing tables
	 * and consider every address is directly reachable on the interface
	 */
	ret = 1;
	ret = setsockopt(nga->sock, SOL_SOCKET, SO_DONTROUTE, &ret, sizeof(ret));
	if (ret <0) {
		perror("setsockopt(SO_DONTROUTE)");
		return ret;
	}
	
	
	return 0;
}


static int checkErrorCode (const struct nsdp_cmd *nc)
{
	switch (nc->error) {
	
	case 0:
		return ERR_OK;
	
	case ERROR_DENIED:
		return (nc->attr_error == ATTR_PASSWORD) ? ERR_BADPASS : ERR_DENIED;
	
	case ERROR_INVALID_VALUE:
		return ERR_INVARG;
	
	default:
		return ERR_UNKNOWN;
	}
}


void prepareSend (struct ngadmin *nga, struct nsdp_cmd *nc, unsigned char code)
{
	struct swi_attr *sa = nga->current;
	
	
	memset(nc, 0, sizeof(struct nsdp_cmd));
	memcpy(&nc->client_mac, &nga->localmac, ETH_ALEN);
	nc->remote_addr.sin_family = AF_INET;
	nc->remote_addr.sin_port = htons(SWITCH_PORT);
	if (sa != NULL)
		memcpy(&nc->switch_mac, &sa->mac, ETH_ALEN);
	
	/* destination address selection */
	if (sa != NULL && !nga->keepbroad)
		nc->remote_addr.sin_addr = sa->nc.ip;
	else if (nga->globalbroad)
		nc->remote_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	else
		nc->remote_addr.sin_addr = nga->brd;
	
	nc->seqnum = ++nga->seq;
	nc->code = code;
}


void prepareRecv (struct ngadmin *nga, struct nsdp_cmd *nc, unsigned char code)
{
	struct swi_attr *sa = nga->current;
	
	
	memset(nc, 0, sizeof(struct nsdp_cmd));
	memcpy(&nc->client_mac, &nga->localmac, ETH_ALEN);
	nc->remote_addr.sin_family = AF_INET;
	nc->remote_addr.sin_port = htons(SWITCH_PORT);
	if (sa != NULL)
		memcpy(&nc->switch_mac, &sa->mac, ETH_ALEN);
	
	/* set filter on switch IP */
	if (sa == NULL)
		nc->remote_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	else
		nc->remote_addr.sin_addr = sa->nc.ip;
	
	nc->seqnum = nga->seq;
	nc->code = code;
}


int readRequest (struct ngadmin *nga, List *attr)
{
	int i, ret = ERR_OK;
	struct nsdp_cmd nc;
	
	
	if (nga == NULL) {
		ret = ERR_INVARG;
		goto end;
	}
	
	/* add end attribute to end */
	pushBackList(attr, newEmptyAttr(ATTR_END));
	
	prepareSend(nga, &nc, CODE_READ_REQ);
	i = sendNsdpPacket(nga->sock, &nc, attr);
	
	/* do not destroy the list, it will be filled again later by recvNsdpPacket */
	clearList(attr, (void(*)(void*))freeAttr);
	
	if (i >= 0) {
		prepareRecv(nga, &nc, CODE_READ_REP);
		i = recvNsdpPacket(nga->sock, &nc, attr, &nga->timeout);
	}
	
	if (i == -EINVAL) {
		ret = ERR_INVARG;
		goto end;
	} else if (i < 0) {
		ret = (errno == EAGAIN || errno == EWOULDBLOCK) ? ERR_TIMEOUT : ERR_NET;
		goto end;
	}
	
	
	/* check the switch error code */
	ret = checkErrorCode(&nc);
	
	
end:
	return ret;
}


int writeRequest (struct ngadmin *nga, List *attr)
{
	int i, ret = ERR_OK;
	struct attr *at;
	struct nsdp_cmd nc;
	
	
	if (nga == NULL) {
		ret = ERR_INVARG;
		goto end;
	} else if (nga->current == NULL) {
		ret = ERR_NOTLOG;
		goto end;
	}
	
	
	if (attr == NULL)
		attr = createEmptyList();
	
	/* add password attribute to start */
	at = newAttr(ATTR_PASSWORD, strlen(nga->password), strdup(nga->password));
	if (nga->encrypt_pass)
		passwordEndecode(at->data, at->size);
	pushFrontList(attr, at);
	
	/* add end attribute to end */
	pushBackList(attr, newEmptyAttr(ATTR_END));
	
	prepareSend(nga, &nc, CODE_WRITE_REQ);
	i = sendNsdpPacket(nga->sock, &nc, attr);
	
	/* the list will be filled again by recvNgPacket
	 * but normally it will be still empty
	 */
	clearList(attr, (void(*)(void*))freeAttr);
	
	if (i >= 0) {
		prepareRecv(nga, &nc, CODE_WRITE_REP);
		i = recvNsdpPacket(nga->sock, &nc, attr, &nga->timeout);
	}
	
	if (i == -EINVAL) {
		ret = ERR_INVARG;
		goto end;
	} else if (i < 0) {
		ret = (errno == EAGAIN || errno == EWOULDBLOCK) ? ERR_TIMEOUT : ERR_NET;
		goto end;
	}
	
	/* check the switch error code */
	ret = checkErrorCode(&nc);
	
	
end:
	/* the switch replies to write request by just a header (no attributes), so the list can be destroyed */
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int extractSwitchAttributes (struct swi_attr *sa, const List *l)
{
	const ListNode *ln;
	const struct attr *at;
	int len;
	
	
	memset(sa, 0, sizeof(struct swi_attr));
	
	for (ln = l->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		if (at->size == 0)
			return -EMSGSIZE;
		
		switch (at->attr) {
		
		case ATTR_PRODUCT:
			len = min(at->size, PRODUCT_SIZE);
			memcpy(sa->product, at->data, len);
			trim(sa->product, len);
			break;
		
		case ATTR_NAME:
			len = min(at->size, NAME_SIZE);
			memcpy(sa->name, at->data, len);
			trim(sa->name, len);
			break;
		
		case ATTR_MAC:
			memcpy(&sa->mac, at->data, ETH_ALEN);
			break;
		
		case ATTR_IP:
			sa->nc.ip = *(struct in_addr*)at->data;
			break;
		
		case ATTR_NETMASK:
			sa->nc.netmask = *(struct in_addr*)at->data;
			break;
		
		case ATTR_GATEWAY:
			sa->nc.gw = *(struct in_addr*)at->data;
			break;
		
		case ATTR_DHCP:
			/* Note: DHCP attribute is special, it is 2 two bytes long when sent
			 * by the switch but only 1 byte long when sent by the client
			 */
			sa->nc.dhcp = (at->size == 2) && ((*(unsigned short*)at->data) == 1);
			break;
		
		case ATTR_FIRM_VER:
			len = min(at->size, FIRMWARE_SIZE - 1);
			memcpy(sa->firmware, at->data, len);
			sa->firmware[len] = '\0';
			break;
		
		case ATTR_PORTS_COUNT:
			sa->ports = *(unsigned char*)at->data;
			break;
		
		case ATTR_END:
			return 0;
		}
	}
	
	
	return 0;
}


