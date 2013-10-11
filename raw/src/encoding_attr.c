
#include <ngadmin.h> /* FIXME */
#include <nsdp/protocol.h>
#include "encoding_attr.h"


#define UNUSED			__attribute__((unused))
#define ATTR_HANDLER_ENTRY(at, sz, enc, dec)	{.attr = at, .size = sz, .encode = enc, .decode = dec}



static bool bool_endecode (struct attr *at, unsigned char ports UNUSED)
{
	*(char*)at->data = (*(char*)at->data != 0);
	
	return true;
}


static bool ports_status_decode (struct attr *at, unsigned char ports)
{
	struct attr_port_status *ps = at->data;
	
	if (ps->port < 1 || ps->port > ports)
		return false;
	
	switch (ps->status) {
	case SPEED_DOWN:
	case SPEED_10:
	case SPEED_100:
	case SPEED_1000:
		break;
	default:
		return false;
	}
	
	return true;
}


static bool port_stat_decode (struct attr *at, unsigned char ports)
{
	struct attr_port_stat *ps = at->data;
	unsigned long long *v;
	
	if (ps->port < 1 || ps->port > ports)
		return false;
	
	for (v = &ps->recv; ((char*)v) - ((char*)ps) < (int)sizeof(struct attr_port_stat); v++)
		*v = be64toh(*v);
	
	
	return true;
}


static bool bitrate_decode (struct attr *at, unsigned char ports)
{
	struct attr_bitrate *sb = at->data;
	
	if (sb->port < 1 || sb->port > ports)
		return false;
	
	sb->bitrate = ntohl(sb->bitrate);
	if (sb->bitrate < BITRATE_UNSPEC || sb->bitrate > BITRATE_512M)
		return false;
	
	return true;
}


static bool bitrate_encode (struct attr *at, unsigned char ports)
{
	struct attr_bitrate *sb = at->data;
	
	if (sb->port < 1 || sb->port > ports)
		return false;
	
	if (sb->bitrate < BITRATE_UNSPEC || sb->bitrate > BITRATE_512M)
		return false;
	sb->bitrate = htonl(sb->bitrate);
	
	return true;
}


static bool qos_mode_endecode (struct attr *at, unsigned char ports UNUSED)
{
	unsigned char v = *(char*)at->data;
	
	return (v == QOS_PORT || v == QOS_DOT);
}


static bool qos_endecode (struct attr *at, unsigned char ports)
{
	struct attr_qos *aq = at->data;
	
	if (aq->port < 1 || aq->port > ports)
		return false;
	
	return (aq->prio >= PRIO_HIGH && aq->prio <= PRIO_LOW);
}


static bool pvid_decode (struct attr *at, unsigned char ports)
{
	struct attr_pvid *ap= at->data;
	
	if (ap->port < 1 || ap->port > ports)
		return false;
	
	ap->vlan = ntohs(ap->vlan);
	
	return (ap->vlan >= VLAN_MIN && ap->vlan <= VLAN_DOT_MAX);
}


static bool pvid_encode (struct attr *at, unsigned char ports)
{
	struct attr_pvid *ap= at->data;
	
	if (ap->port < 1 || ap->port > ports)
		return false;
	
	if (ap->vlan < VLAN_MIN || ap->vlan > VLAN_DOT_MAX)
		return false;
	
	ap->vlan = htons(ap->vlan);
	
	return true;
}


static bool vlan_destroy_encode (struct attr *at, unsigned char ports UNUSED)
{
	unsigned short v = *(unsigned short*)at->data;
	
	if (v < VLAN_MIN || v > VLAN_DOT_MAX)
		return false;
	
	*(unsigned short*)at->data = htons(v);
	
	return true;
}


static bool mirror_decode (struct attr *at, unsigned char ports)
{
	unsigned char *r = at->data, *p;
	int port;
	
	
	if (at->size != 3 + ((ports - 1) >> 3))
		return false;
	
	/* r[0] == 0 is allowed and means mirroring is disabled */
	if (r[0] > ports)
		return false;
	
	p = malloc(1 + ports);
	if (p == NULL)
		return false;
	
	memset(p, 0, 1 + ports);
	
	if (r[0] == 0)
		goto end;
	
	p[0] = r[0];
	
	for (port = 1; port <= ports; port++)
		p[port] = (r[2] >> (8 - port)) & 1; /* FIXME: if ports > 8 */
	
end:
	free(at->data);
	at->data = p;
	at->size = 1 + ports;
	
	
	return true;
}


static bool mirror_encode (struct attr *at, unsigned char ports)
{
	unsigned char *p = at->data, *r;
	int port;
	
	
	if (at->size != 1 + ports)
		return false;
	
	/* p[0] == 0 is allowed and means mirroring is disabled */
	if (p[0] > ports)
		return false;
	
	r = malloc(3 + ((ports - 1) >> 3));
	if (r == NULL)
		return false;
	
	memset(r, 0, 3 + ((ports - 1) >> 3));
	
	if (p[0] == 0)
		goto end;
	
	r[0] = p[0];
	
	for (port = 1; port <= ports; port++) {
		if (p[0] != port)
			r[2] |= (p[port] & 1) << (8 - port); /* FIXME: if ports > 8 */
	}
	
end:
	free(at->data);
	at->data = r;
	at->size = 3 + ((ports - 1) >> 3);
	
	
	return true;
}


static bool igmp_vlan_decode (struct attr *at, unsigned char ports UNUSED)
{
	struct attr_igmp_vlan *aiv = at->data;
	
	aiv->enable = ntohs(aiv->enable);
	if (aiv->enable != 0 && aiv->enable != 1)
		return false;
	
	aiv->vlan = ntohs(aiv->vlan);
	if (aiv->vlan < VLAN_MIN || aiv->vlan > VLAN_DOT_MAX)
		return false;
	
	return true;
}


static bool igmp_vlan_encode (struct attr *at, unsigned char ports UNUSED)
{
	struct attr_igmp_vlan *aiv = at->data;
	
	if (aiv->enable != 0 && aiv->enable != 1)
		return false;
	aiv->enable = htons(aiv->enable);
	
	if (aiv->vlan < VLAN_MIN || aiv->vlan > VLAN_DOT_MAX)
		return false;
	aiv->vlan = htons(aiv->vlan);
	
	return true;
}


static bool cabletest_do_encode (struct attr *at, unsigned char ports)
{
	struct attr_cabletest_do *acd = at->data;
	
	if (acd->port < 1 || acd->port > ports)
		return false;
	
	return (acd->action == 1);
}


static bool cabletest_result_encode (struct attr *at, unsigned char ports)
{
	unsigned char v = *(unsigned char*)at->data;
	
	return (v >= 1 && v <= ports);
}


static bool cabletest_result_decode (struct attr *at, unsigned char ports)
{
	struct attr_cabletest_result *acr = at->data;
	
	if (acr->port < 1 || acr->port > ports)
		return false;
	
	acr->v1 = ntohl(acr->v1);
	acr->v2 = ntohl(acr->v2);
	
	return true;
}


static bool cabletest_result_endecode (struct attr *at, unsigned char ports)
{
	switch (at->size) {
	
	case 1:
		return cabletest_result_encode(at, ports);
	
	case sizeof(struct attr_cabletest_result):
		return cabletest_result_decode(at, ports);
	
	default:
		return false;
	}
}


static bool vlan_type_endecode (struct attr *at, unsigned char ports UNUSED)
{
	char v = *(char*)at->data;
	
	return (v >= VLAN_DISABLED && v <= VLAN_DOT_ADV);
}


static bool vlan_port_decode (struct attr *at, unsigned char ports)
{
	char *r = at->data;
	struct attr_vlan_conf *avc;
	int port;
	
	
	if (at->size != (2 + 1 + ((ports - 1) >> 3)))
		return false;
	
	avc = malloc(sizeof(struct attr_vlan_conf) + ports);
	if (avc == NULL)
		return false;
	
	avc->vlan = ntohs(*(unsigned short*)r);
	r += 2;
	
	for (port = 0; port < ports; port++) {
		/* FIXME: if ports > 8 */
		if ((r[0] >> (7 - port)) & 1)
			avc->ports[port] = VLAN_UNTAGGED;
		else
			avc->ports[port] = VLAN_NO;
	}
	
	free(at->data);
	at->data = avc;
	at->size = sizeof(struct attr_vlan_conf) + ports;
	
	
	return true;
}


static bool vlan_port_encode (struct attr *at, unsigned char ports)
{
	struct attr_vlan_conf *avc = at->data;
	char *r;
	unsigned int size, port;
	
	
	if (avc->vlan < VLAN_MIN || avc->vlan > VLAN_PORT_MAX)
		return false;
	
	/* just a header is valid */
	if (at->size == sizeof(struct attr_vlan_conf))
		size = 2;
	else if (at->size == sizeof(struct attr_vlan_conf) + ports)
		size = (2 + 1 + ((ports - 1) >> 3));
	else
		return false;
	
	r = malloc(size);
	if (r == NULL)
		return false;
	
	memset(r, 0, size);
	*(unsigned short*)r = htons(avc->vlan);
	
	if (size == 2)
		goto end;
	
	r += 2;

	for (port = 0; port < ports; port++) {
		/* FIXME: if ports > 8 */
		if (avc->ports[port] == VLAN_UNTAGGED)
			r[0] |= (1 << (7 - port));
	}

	r -= 2;
	
end:
	free(at->data);
	at->data = r;
	at->size = size;
	
	
	return true;
}


static bool vlan_dot_decode (struct attr *at, unsigned char ports)
{
	char *r = at->data;
	struct attr_vlan_conf *avc;
	int port;
	
	
	if (at->size != (2 + 2 * (1 + ((ports - 1) >> 3))))
		return false;
	
	avc = malloc(sizeof(struct attr_vlan_conf) + ports);
	if (avc == NULL)
		return false;
	
	avc->vlan = ntohs(*(unsigned short*)r);
	r += 2;
	
	for (port = 0; port < ports; port++) {
		/* FIXME: if ports > 8 */
		if ((r[1] >> (7 - port)) & 1)
			avc->ports[port] = VLAN_TAGGED;
		else if ((r[0] >> (7 - port)) & 1)
			avc->ports[port] = VLAN_UNTAGGED;
		else
			avc->ports[port] = VLAN_NO;
	}
	
	free(at->data);
	at->data = avc;
	at->size = sizeof(struct attr_vlan_conf) + ports;
	
	
	return true;
}


static bool vlan_dot_encode (struct attr *at, unsigned char ports)
{
	struct attr_vlan_conf *avc = at->data;
	char *r, fl;
	unsigned int size, port;
	
	
	if (avc->vlan < VLAN_MIN || avc->vlan > VLAN_DOT_MAX)
		return false;
	
	/* just a header is valid */
	if (at->size == sizeof(struct attr_vlan_conf))
		size = 2;
	else if (at->size == sizeof(struct attr_vlan_conf) + ports)
		size = (2 + 2 * (1 + ((ports - 1) >> 3)));
	else
		return false;
	
	r = malloc(size);
	if (r == NULL)
		return false;
	
	memset(r, 0, size);
	*(unsigned short*)r = htons(avc->vlan);
	
	if (size == 2)
		goto end;
	
	r += 2;
	
	for (port = 0; port < ports; port++) {
		/* FIXME: if ports > 8 */
		fl = (1 << (7 - port));
		switch (avc->ports[port]) {
		case VLAN_TAGGED:
			r[1] |= fl;
			/* a tagged VLAN is also marked as untagged
			 * so do not put a "break" here */
		case VLAN_UNTAGGED:
			r[0] |= fl;
		}
	}
	
	r -= 2;
	
end:
	free(at->data);
	at->data = r;
	at->size = size;
	
	
	return true;
}


/* WARNING: attributes codes MUST be in ascending order */
static const struct attr_handler attrtab[] = {
	ATTR_HANDLER_ENTRY(ATTR_MAC, 6, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_IP, 4, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_NETMASK, 4, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_GATEWAY, 4, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_DHCP, 2, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_RESTART, 1, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_ENCPASS, 4, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_DEFAULTS, 1, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_PORT_STATUS, sizeof(struct attr_port_status), NULL, ports_status_decode),
	ATTR_HANDLER_ENTRY(ATTR_PORT_STATISTICS, sizeof(struct attr_port_stat), NULL, port_stat_decode),
	ATTR_HANDLER_ENTRY(ATTR_STATS_RESET, 1, bool_endecode, bool_endecode),
	ATTR_HANDLER_ENTRY(ATTR_CABLETEST_DO, sizeof(struct attr_cabletest_do), cabletest_do_encode, NULL),
	ATTR_HANDLER_ENTRY(ATTR_CABLETEST_RESULT, 0, cabletest_result_endecode, cabletest_result_endecode),
	ATTR_HANDLER_ENTRY(ATTR_VLAN_TYPE, 1, vlan_type_endecode, vlan_type_endecode),
	ATTR_HANDLER_ENTRY(ATTR_VLAN_PORT_CONF, 0, vlan_port_encode, vlan_port_decode),
	ATTR_HANDLER_ENTRY(ATTR_VLAN_DOT_CONF, 0, vlan_dot_encode, vlan_dot_decode),
	ATTR_HANDLER_ENTRY(ATTR_VLAN_DESTROY, 2, vlan_destroy_encode, NULL),
	ATTR_HANDLER_ENTRY(ATTR_VLAN_PVID, sizeof(struct attr_pvid), pvid_encode, pvid_decode),
	ATTR_HANDLER_ENTRY(ATTR_QOS_TYPE, 1, qos_mode_endecode, qos_mode_endecode),
	ATTR_HANDLER_ENTRY(ATTR_QOS_CONFIG, sizeof(struct attr_qos), qos_endecode, qos_endecode),
	ATTR_HANDLER_ENTRY(ATTR_BITRATE_INPUT, sizeof(struct attr_bitrate), bitrate_encode, bitrate_decode),
	ATTR_HANDLER_ENTRY(ATTR_BITRATE_OUTPUT, sizeof(struct attr_bitrate), bitrate_encode, bitrate_decode),
	ATTR_HANDLER_ENTRY(ATTR_STORM_ENABLE, 1, bool_endecode, bool_endecode),
	ATTR_HANDLER_ENTRY(ATTR_STORM_BITRATE, sizeof(struct attr_bitrate), bitrate_encode, bitrate_decode),
	ATTR_HANDLER_ENTRY(ATTR_MIRROR, 0, mirror_encode, mirror_decode),
	ATTR_HANDLER_ENTRY(ATTR_PORTS_COUNT, 1, NULL, NULL),
	ATTR_HANDLER_ENTRY(ATTR_IGMP_ENABLE_VLAN, sizeof(struct attr_igmp_vlan), igmp_vlan_encode, igmp_vlan_decode),
	ATTR_HANDLER_ENTRY(ATTR_IGMP_BLOCK_UNK, 1, bool_endecode, bool_endecode),
	ATTR_HANDLER_ENTRY(ATTR_IGMP_VALID_V3, 1, bool_endecode, bool_endecode)
};


const struct attr_handler* getAttrHandler (unsigned short attrcode)
{
	const struct attr_handler *ah;
	const unsigned int tab_size = sizeof(attrtab) / sizeof(struct attr_handler);
	unsigned int inf, sup, index;
	
	
	inf = 0;
	sup = tab_size;
	index = tab_size >> 1;
	while (index < sup) {
		ah = &attrtab[index];
		
		if (ah->attr > attrcode) {
			sup = index;
			index -= ((index - inf) >> 1) + ((index - inf) & 1);
		} else if (ah->attr < attrcode) {
			inf = index;
			index += ((sup - index) >> 1) + ((sup - index) & 1);
		} else {
			return ah;
		}
	}
	
	
	return NULL;
}


