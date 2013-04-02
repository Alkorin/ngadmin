
#include <errno.h>

#include <ngadmin.h>

#include <attr.h>
#include <protocol.h>

#include "lib.h"
#include "network.h"


static const struct timeval default_timeout = {.tv_sec = 4, .tv_usec = 0};



struct ngadmin* ngadmin_init (const char *iface)
{
	struct ngadmin *nga;
	
	
	/* allocate main structure */
	nga = malloc(sizeof(struct ngadmin));
	memset(nga, 0, sizeof(struct ngadmin));
	
	strncpy(nga->iface, iface, IFNAMSIZ - 1);
	
	if (startNetwork(nga) < 0) {
		free(nga);
		return NULL;
	}
	
	nga->timeout = default_timeout;
	if (updateTimeout(nga) < 0) {
		free(nga);
		return NULL;
	}
	
	
	return nga;
}


int ngadmin_close (struct ngadmin *nga)
{
	if (nga == NULL)
		return ERR_INVARG;
		
	stopNetwork(nga);
	free(nga->swi_tab);
	free(nga);
	
	return ERR_OK;
}


int ngadmin_forceInterface (struct ngadmin *nga)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	return forceInterface(nga) == 0 ? ERR_OK : ERR_NET;
}


int ngadmin_setKeepBroadcasting (struct ngadmin *nga, bool value)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	nga->keepbroad = value;
	
	return ERR_OK;
}


int ngadmin_useGlobalBroadcast (struct ngadmin *nga, bool value)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	nga->globalbroad = value;
	
	return ERR_OK;
}


int ngadmin_setPassword (struct ngadmin *nga, const char *pass)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	strncpy(nga->password, pass, PASSWORD_MAX);
	
	return ERR_OK;
}


int ngadmin_setTimeout (struct ngadmin *nga, const struct timeval *tv)
{
	int ret = ERR_OK;
	
	
	if (nga == NULL || tv == NULL)
		return ERR_INVARG;
	
	nga->timeout = *tv;
	if (updateTimeout(nga) < 0)
		ret = ERR_NET;
	
	
	return ret;
}


int ngadmin_scan (struct ngadmin *nga)
{
	int i;
	List *attr, *swiList;
	struct swi_attr *sa;
	/*
	sent by official win client:
	ATTR_PRODUCT,
	ATTR_UNK2,
	ATTR_NAME,
	ATTR_MAC,
	ATTR_UNK5,
	ATTR_IP,
	ATTR_NETMASK,
	ATTR_GATEWAY,
	ATTR_DHCP,
	ATTR_UNK12,
	ATTR_FIRM_VER,
	ATTR_UNK14,
	ATTR_UNK15,
	ATTR_END
	*/
	static const unsigned short hello[] = {
		ATTR_PRODUCT,
		ATTR_NAME,
		ATTR_MAC,
		ATTR_IP,
		ATTR_NETMASK,
		ATTR_GATEWAY,
		ATTR_DHCP,
		ATTR_FIRM_VER,
		ATTR_PORTS_COUNT,
		ATTR_END
	};
	
	
	if (nga == NULL)
		return ERR_INVARG;
	
	free(nga->swi_tab);
	nga->swi_tab = NULL;
	nga->swi_count = 0;
	nga->current = NULL;
	
	
	/* create attributes for an "hello" request */
	attr = createEmptyList();
	for (i = 0; ; i++) {
		pushBackList(attr, newEmptyAttr(hello[i]));
		if (hello[i] == ATTR_END)
			break;
	}
	
	/* send request to all potential switches */
	i = sendNgPacket(nga, CODE_READ_REQ, attr);
	clearList(attr, (void(*)(void*))freeAttr);
	if (i == -EINVAL)
		return ERR_INVARG;
	else if (i < 0)
		return ERR_NET;
	
	/* try to receive any packets until timeout */
	swiList = createEmptyList();
	/* FIXME: end after timeout whatever received packet is good or not */
	while (recvNgPacket(nga, CODE_READ_REP, NULL, NULL, attr) >= 0) {
		sa = malloc(sizeof(struct swi_attr));
		if (sa == NULL)
			return ERR_MEM;
		extractSwitchAttributes(sa, attr);
		clearList(attr, (void(*)(void*))freeAttr);
		pushBackList(swiList, sa);
	}
	
	nga->swi_count = swiList->count;
	nga->swi_tab = convertToArray(swiList, sizeof(struct swi_attr));
	destroyList(swiList, free);
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ERR_OK;
}


const struct swi_attr* ngadmin_getSwitchTab (struct ngadmin *nga, int *nb)
{
	if (nga == NULL || nb == NULL)
		return NULL;
	
	*nb = nga->swi_count;
	
	return nga->swi_tab;
}


const struct swi_attr* ngadmin_getCurrentSwitch (struct ngadmin *nga)
{
	if (nga == NULL)
		return NULL;
	
	return nga->current;
}


int ngadmin_login (struct ngadmin *nga, int id)
{
	List *attr;
	int ret = ERR_OK;
	struct swi_attr *sa;
	
	
	if (nga == NULL)
		return ERR_INVARG;
	else if (id < 0 || id >= nga->swi_count)
		return ERR_BADID;
	
	sa = &nga->swi_tab[id];
	nga->current = sa;
	
	attr = createEmptyList();
	pushBackList(attr, newAttr(ATTR_PASSWORD, strlen(nga->password), strdup(nga->password)));
	ret = readRequest(nga, attr);
	if (ret == ERR_OK ) {
		/* login succeeded */
		/* TODO: if keep broadcasting is disabled, connect() the UDP 
		socket so icmp errors messages (port unreachable, TTL exceeded 
		in transit, ...) can be received */
	} else {
		/* login failed */
		nga->current = NULL;
	}
	
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_upgradeFirmware (struct ngadmin *nga, const char *filename)
{
	if (nga == NULL || filename == NULL || *filename == 0)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	/*
	Firmware upgrade is not yet implemented. 
	This would require much more work and the use of a TFTP client. 
	Overall, it could be quite dangerous, as the switch may not check the binary 
	content sent to it. 
	*/
	
	return ERR_NOTIMPL;
}


int ngadmin_getPortsStatus (struct ngadmin *nga, unsigned char *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_port_status *ps;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_PORT_STATUS));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_PORT_STATUS, ATTR_END);
	
	memset(ports, SPEED_UNK, nga->current->ports);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		ps = at->data;
		ports[ps->port - 1] = ps->status;
	}
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setName (struct ngadmin *nga, const char *name)
{
	List *attr;
	int ret = ERR_OK;
	
	
	if (nga == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, name == NULL ? newEmptyAttr(ATTR_NAME) : newAttr(ATTR_NAME, strlen(name), strdup(name)) );
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	 
	/* successful, also update local name */
	if (name == NULL)
		memset(nga->current->name, '\0', NAME_SIZE);
	else
		strncpy(nga->current->name, name, NAME_SIZE);
	
end:
	return ret;
}


int ngadmin_getPortsStatistics (struct ngadmin *nga, struct port_stats *ps)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_port_stat *aps;
	
	
	if (nga == NULL || ps == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_PORT_STATISTICS));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_PORT_STATISTICS, ATTR_END);
	
	memset(ps, 0, nga->current->ports * sizeof(struct port_stats));
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		aps = at->data;
		ps[aps->port -1].recv = aps->recv;
		ps[aps->port -1].sent = aps->sent;
		ps[aps->port -1].crc = aps->crc;
	}
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_resetPortsStatistics (struct ngadmin *nga)
{
	List *attr;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_STATS_RESET, 1));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_changePassword (struct ngadmin *nga, const char* pass)
{
	List *attr;
	int ret = ERR_OK;
	
	
	if (nga == NULL || pass == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newAttr(ATTR_NEW_PASSWORD, strlen(pass), strdup(pass)));
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	
	/* successful, also update local password */
	strncpy(nga->password, pass, PASSWORD_MAX);
	
end:
	
	return ret;
}


int ngadmin_getStormFilterState (struct ngadmin *nga, int *s)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	
	
	if (nga == NULL || s == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_STORM_ENABLE));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_STORM_ENABLE, ATTR_END);
	
	*s = 0;
	
	if (attr->first != NULL) {
		at = attr->first->data;
		*s = *(char*)at->data;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setStormFilterState (struct ngadmin *nga, int s)
{
	List *attr;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_STORM_ENABLE, s != 0));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getStormFilterValues (struct ngadmin *nga, int *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK, port;
	struct attr_bitrate *sb;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_STORM_BITRATE));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_STORM_BITRATE, ATTR_END);
	
	for (port = 0; port < nga->current->ports; port++)
		ports[port] = BITRATE_UNSPEC;
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		sb = at->data;
		ports[sb->port - 1] = sb->bitrate;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setStormFilterValues (struct ngadmin *nga, const int *ports)
{
	List *attr;
	int port;
	struct attr_bitrate *sb;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	
	for (port = 0; port < nga->current->ports; port++) {
		if (ports[port] != BITRATE_UNSPEC) {
			sb = malloc(sizeof(struct attr_bitrate));
			if (sb == NULL)
				return ERR_MEM;
			sb->port = port + 1;
			sb->bitrate = ports[port];
			pushBackList(attr, newAttr(ATTR_STORM_BITRATE, sizeof(struct attr_bitrate), sb));
		}
	}
	
	return writeRequest(nga, attr);
}


int ngadmin_getBitrateLimits (struct ngadmin *nga, int *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK, port;
	struct attr_bitrate *pb;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_BITRATE_INPUT));
	pushBackList(attr, newEmptyAttr(ATTR_BITRATE_OUTPUT));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	
	for (port = 0; port < nga->current->ports; port++) {
		ports[2 * port + 0] = BITRATE_UNSPEC;
		ports[2 * port + 1] = BITRATE_UNSPEC;
	}
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		pb = at->data;
		if (at->attr == ATTR_BITRATE_INPUT)
			ports[(pb->port - 1) * 2 + 0] = pb->bitrate;
		else if (at->attr == ATTR_BITRATE_OUTPUT)
			ports[(pb->port - 1) * 2 + 1] = pb->bitrate;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	return ret;
}


int ngadmin_setBitrateLimits (struct ngadmin *nga, const int *ports)
{
	List *attr;
	int port;
	struct attr_bitrate *pb;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	
	for (port = 0; port < nga->current->ports; port++) {
		if (ports[2 * port + 0] >= BITRATE_NOLIMIT && ports[2 * port + 0] <= BITRATE_512M) {
			pb = malloc(sizeof(struct attr_bitrate));
			if (pb == NULL)
				return ERR_MEM;
			pb->port = port + 1;
			pb->bitrate = ports[2 * port + 0];
			pushBackList(attr, newAttr(ATTR_BITRATE_INPUT, sizeof(struct attr_bitrate), pb));
		}
		if (ports[2 * port + 1] >= BITRATE_NOLIMIT && ports[2 * port + 1] <= BITRATE_512M) {
			pb = malloc(sizeof(struct attr_bitrate));
			if (pb == NULL)
				return ERR_MEM;
			pb->port = port + 1;
			pb->bitrate = ports[2 * port + 1];
			pushBackList(attr, newAttr(ATTR_BITRATE_OUTPUT, sizeof(struct attr_bitrate), pb));
		}
	}
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getQOSMode (struct ngadmin *nga, int *s)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	
	
	if (nga == NULL || s == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_QOS_TYPE));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_QOS_TYPE, ATTR_END);
	
	*s = 0;
	
	if (attr->first != NULL) {
		at = attr->first->data;
		*s = *(char*)at->data;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setQOSMode (struct ngadmin *nga, int s)
{
	List *attr;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_QOS_TYPE, s));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getQOSValues (struct ngadmin *nga, char *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK, port;
	struct attr_qos *aq;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_QOS_CONFIG));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_QOS_CONFIG, ATTR_END);
	
	for (port = 0; port < nga->current->ports; port++)
		ports[port] = PRIO_UNSPEC;
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		aq = at->data;
		ports[aq->port - 1] = aq->prio;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setQOSValues (struct ngadmin *nga, const char *ports)
{
	List *attr;
	int port;
	struct attr_qos *aq;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	
	for (port = 0; port < nga->current->ports; port++) {
		if (ports[port] >= PRIO_HIGH && ports[port] <= PRIO_LOW) {
			aq = malloc(sizeof(struct attr_qos));
			if (aq == NULL)
				return ERR_MEM;
			aq->port = port + 1;
			aq->prio = ports[port];
			pushBackList(attr, newAttr(ATTR_QOS_CONFIG, sizeof(struct attr_qos), aq));
		}
	}
	
	
	return writeRequest(nga, attr);
}


int ngadmin_restart (struct ngadmin *nga)
{
	List *attr;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_RESTART, 1));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_defaults (struct ngadmin *nga)
{
	List *attr;
	int ret = ERR_OK;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_DEFAULTS, 1));
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	
	/* successful: delog and clean list */
	free(nga->swi_tab);
	nga->swi_tab = NULL;
	nga->swi_count = 0;
	nga->current = NULL;
	
end:
	return ret;
}


int ngadmin_getMirror (struct ngadmin *nga, char *ports)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_MIRROR));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_MIRROR, ATTR_END);
	
	memset(ports, 0, 1 + nga->current->ports);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		memcpy(ports, at->data, 1 + nga->current->ports);
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setMirror (struct ngadmin *nga, const char *ports)
{
	List *attr;
	char *p;
	
	
	if (nga == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	p = malloc(1 + nga->current->ports);
	if (p == NULL)
		return ERR_MEM;
	
	if (ports == NULL)
		memset(p, 0, 1 + nga->current->ports);
	else
		memcpy(p, ports, 1 + nga->current->ports);
	
	attr = createEmptyList();
	pushBackList(attr, newAttr(ATTR_MIRROR, 1 + nga->current->ports, p));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getIGMPConf (struct ngadmin *nga, struct igmp_conf *ic)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_igmp_vlan *aiv;
	
	
	if (nga == NULL || ic == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	/*
	ATTR_IGMP_ENABLE_VLAN
	ATTR_IGMP_BLOCK_UNK
	ATTR_IGMP_VALID_V3
	
	Apparently, read-querying these attributes at the same time causes the switch to reply garbage. 
	Here we are forced to do like the official win app and send a separate request for each attribute. 
	*/
	
	
	attr = createEmptyList();
	memset(ic, 0, sizeof(struct igmp_conf));
	
	
	pushBackList(attr, newEmptyAttr(ATTR_IGMP_ENABLE_VLAN));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_IGMP_ENABLE_VLAN, ATTR_END);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		aiv = at->data;
		ic->enable = aiv->enable;
		ic->vlan = aiv->vlan;
	}
	
	clearList(attr, (void(*)(void*))freeAttr);
	
	
	pushBackList(attr, newEmptyAttr(ATTR_IGMP_BLOCK_UNK));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_IGMP_BLOCK_UNK, ATTR_END);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		ic->block = *(char*)at->data;
	}
	
	clearList(attr, (void(*)(void*))freeAttr);
	
	
	pushBackList(attr, newEmptyAttr(ATTR_IGMP_VALID_V3));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_IGMP_VALID_V3, ATTR_END);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		ic->validate = *(char*)at->data;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setIGMPConf (struct ngadmin *nga, const struct igmp_conf *ic)
{
	List *attr;
	struct attr_igmp_vlan *aiv;
	
	
	if (nga == NULL || ic == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	aiv = malloc(sizeof(struct attr_igmp_vlan));
	if (aiv == NULL)
		return ERR_MEM;
	aiv->enable = ic->enable;
	aiv->vlan = ic->vlan;
	
	
	attr = createEmptyList();
	pushBackList(attr, newAttr(ATTR_IGMP_ENABLE_VLAN, sizeof(struct attr_igmp_vlan), aiv));
	pushBackList(attr, newByteAttr(ATTR_IGMP_BLOCK_UNK, ic->block != false));
	pushBackList(attr, newByteAttr(ATTR_IGMP_VALID_V3, ic->validate != false));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_cabletest (struct ngadmin *nga, struct cabletest *ct, int nb)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK, i;
	struct attr_cabletest_do *acd;
	struct attr_cabletest_result *acr;
	
	
	if (nga == NULL || ct == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	
	for (i = 0; i < nb; i++) {
		
		acd = malloc(sizeof(struct attr_cabletest_do));
		if (acd == NULL)
			return ERR_MEM;
		acd->port = ct[i].port;
		acd->action = 1;
		pushBackList(attr, newAttr(ATTR_CABLETEST_DO, sizeof(struct attr_cabletest_do), acd));
		
		ret = writeRequest(nga, attr);
		attr = NULL;
		if (ret < 0)
			goto end;
		
		/* the list is destroyed by writeRequest, so we need to recreate it */
		attr = createEmptyList();
		pushBackList(attr, newByteAttr(ATTR_CABLETEST_RESULT, ct[i].port));
		ret = readRequest(nga, attr);
		if (ret < 0)
			goto end;
		
		filterAttributes(attr, ATTR_CABLETEST_RESULT, ATTR_END);
		
		for (ln = attr->first; ln != NULL; ln = ln->next) {
			at = ln->data;
			acr = at->data;
			if (at->size == sizeof(struct attr_cabletest_result) && acr->port == ct[i].port) {
				ct[i].v1 = acr->v1;
				ct[i].v2 = acr->v2;
				break;
			}
		}
		
		/* just empty the list, it will be used at next iteration */
		clearList(attr, (void(*)(void*))freeAttr);
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setNetConf (struct ngadmin *nga, const struct net_conf *nc)
{
	List *attr;
	int ret = ERR_OK;
	struct swi_attr *sa;
	
	
	if (nga == NULL || nc == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	
	if (nc->dhcp) {
		pushBackList(attr, newShortAttr(ATTR_DHCP, 1));
	} else {
		pushBackList(attr, newShortAttr(ATTR_DHCP, 0));
		/* only add non-null values */
		if (nc->ip.s_addr != 0)
			pushBackList(attr, newAddrAttr(ATTR_IP, nc->ip));
		if (nc->netmask.s_addr != 0)
			pushBackList(attr, newAddrAttr(ATTR_NETMASK, nc->netmask));
		if (nc->gw.s_addr != 0)
			pushBackList(attr, newAddrAttr(ATTR_GATEWAY, nc->gw));
	}
	
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	
	/* update local values */
	sa->nc.dhcp = nc->dhcp;
	if (!nc->dhcp) {
		if (nc->ip.s_addr !=0)
			sa->nc.ip = nc->ip;
		if (nc->netmask.s_addr != 0)
			sa->nc.netmask = nc->netmask;
		if (nc->gw.s_addr != 0)
			sa->nc.gw = nc->gw;
	}
	
	
end:
	
	return ret;
}


int ngadmin_getVLANType (struct ngadmin *nga, int *t)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	
	
	if (nga == NULL || t == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_VLAN_TYPE));
	ret=readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_TYPE, ATTR_END);
	
	*t = VLAN_DISABLED;
	
	if (attr->first != NULL) {
		at = attr->first->data;
		*t =(int)*(char*)at->data;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setVLANType (struct ngadmin *nga, int t)
{
	List *attr;
	
	
	if (nga == NULL || t < 1 || t > 4)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_VLAN_TYPE, t));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getVLANDotAllConf (struct ngadmin *nga, unsigned short *vlans, unsigned char *ports, int *nb)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK, total;
	struct attr_vlan_dot *avd;
	
	
	if (nga == NULL || vlans == NULL || ports== NULL || nb == NULL || *nb <= 0)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	total = *nb;
	*nb = 0;
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_VLAN_DOT_CONF));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_DOT_CONF, ATTR_END);
	
	memset(vlans, 0, total * sizeof(unsigned short));
	memset(ports, 0, total * nga->current->ports);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		avd = at->data;
		
		*vlans = avd->vlan;
		memcpy(ports, avd->ports, nga->current->ports);
		
		vlans++;
		ports += nga->current->ports;
		(*nb)++;
		
		if (*nb > total)
			break; /* no more room */
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_getVLANDotConf (struct ngadmin *nga, unsigned short vlan, unsigned char *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_vlan_dot *avd;
	
	
	if (nga == NULL || vlan < 1 || vlan > VLAN_MAX || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newShortAttr(ATTR_VLAN_DOT_CONF, vlan));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_DOT_CONF, ATTR_END);
	
	memset(ports, 0, nga->current->ports);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		avd = at->data;
		if (avd->vlan == vlan) {
			memcpy(ports, avd->ports, nga->current->ports);
			break;
		}
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setVLANDotConf (struct ngadmin *nga, unsigned short vlan, const unsigned char *ports)
{
	List *attr = NULL;
	struct attr *at;
	struct swi_attr *sa;
	struct attr_vlan_dot *avd;
	int ret = ERR_OK, port;
	
	
	if (nga == NULL || vlan < 1 || vlan > VLAN_MAX || ports == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	/* if nothing is to be changed, do nothing */
	for (port = 0; port < sa->ports && ports[port] == VLAN_UNSPEC; port++);
	if (port == sa->ports )
		goto end;
	
	
	attr = createEmptyList();
	avd = malloc(sizeof(struct attr_vlan_dot) + sa->ports);
	if (avd == NULL)
		return ERR_MEM;
	
	avd->vlan = vlan;
	
	/* if all is to be changed, we do not need to read old config */
	if (memchr(ports, VLAN_UNSPEC, sa->ports) != NULL) {
		
		pushBackList(attr, newShortAttr(ATTR_VLAN_DOT_CONF, vlan));
		ret = readRequest(nga, attr);
		if (ret != ERR_OK)
			goto end;
		
		filterAttributes(attr, ATTR_VLAN_DOT_CONF, ATTR_END);
		
		if (attr->first != NULL) {
			at = attr->first->data;
			memcpy(avd, at->data, sizeof(struct attr_vlan_dot) + sa->ports);
		}
		
		clearList(attr, (void(*)(void*))freeAttr);
	}
	
	
	/* apply changes */
	for (port = 0; port < sa->ports; port++) {
		if (ports[port] != VLAN_UNSPEC)
			avd->ports[port] = ports[port];
	}
	
	
	pushBackList(attr, newAttr(ATTR_VLAN_DOT_CONF, sizeof(struct attr_vlan_dot) + sa->ports, avd));
	ret = writeRequest(nga, attr);
	attr = NULL;
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_VLANDestroy (struct ngadmin *nga, unsigned short vlan)
{
	List *attr;
	
	
	if (nga == NULL || vlan < 1 || vlan > VLAN_MAX)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newShortAttr(ATTR_VLAN_DESTROY, vlan));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getAllPVID (struct ngadmin *nga, unsigned short *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_pvid *ap;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_VLAN_PVID));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_PVID, ATTR_END);
	
	memset(ports, 0, nga->current->ports * sizeof(unsigned short));
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		ap = at->data;
		ports[ap->port - 1] = ap->vlan;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setPVID (struct ngadmin *nga, unsigned char port, unsigned short vlan)
{
	List *attr;
	struct attr_pvid *ap;
	
	
	if (nga == NULL || port < 1 || vlan < 1 || vlan > VLAN_MAX)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	else if (port > nga->current->ports)
		return ERR_INVARG;
	
	
	attr = createEmptyList();
	ap = malloc(sizeof(struct attr_pvid));
	if (ap == NULL)
		return ERR_MEM;
	ap->port = port;
	ap->vlan = vlan;
	
	pushBackList(attr, newAttr(ATTR_VLAN_PVID, sizeof(struct attr_pvid), ap));
	
	
	return writeRequest(nga, attr);
}



