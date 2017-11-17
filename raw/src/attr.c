
#include <stdarg.h>
#include <errno.h>

#include <ngadmin.h> /* FIXME */
#include <nsdp/attr.h>
#include <nsdp/protocol.h>


struct attr* newAttr (unsigned short attr, unsigned short size, void *data)
{
	struct attr *at;
	
	at = malloc(sizeof(struct attr));
	if (at == NULL)
		return NULL;
	
	at->attr = attr;
	at->size = size;
	at->data = data;
	
	return at;
}


void freeAttr (struct attr *at)
{
	if (at != NULL) {
		free(at->data);
		free(at);
	}
}


void filterAttributes (List *attr, ...)
{
	va_list ap;
	ListNode *ln, *pr;
	struct attr *at;
	unsigned short attrcode;
	bool keep;
	
	
	for (ln = attr->first; ln != NULL; ) {
		at = ln->data;
		
		va_start(ap, attr);
		keep = false;
		attrcode = 0;
		while (!keep && attrcode != ATTR_END) {
			attrcode = (unsigned short)va_arg(ap, unsigned int);
			keep = keep || (at->attr == attrcode);
		}
		va_end(ap);
		
		if (keep) {
			ln = ln->next;
		} else {
			pr = ln;
			ln = ln->next;
			destroyElement(attr, pr, (void(*)(void*))freeAttr);
		}
	}
}


static int ports_status_endecode (struct attr *at)
{
	struct attr_port_status *ps = at->data;
	
	if (at->size != sizeof(struct attr_port_status))
		return -EMSGSIZE;
	
	if (ps->port < 1)
		return -EINVAL;
	
	switch (ps->status) {
	
	case SPEED_DOWN:
	case SPEED_10_HD:
	case SPEED_10_FD:
	case SPEED_100_HD:
	case SPEED_100_FD:
	case SPEED_1000_FD:
		return 0;
	
	default:
		return -EINVAL;
	}
}


static int port_stat_endecode (struct attr *at, bool encode)
{
	struct attr_port_stat *ps = at->data;
	unsigned long long *v;
	
	if (at->size != sizeof(struct attr_port_stat))
		return -EMSGSIZE;
	
	if (ps->port < 1)
		return -EINVAL;
	
	for (v = &ps->recv; ((void*)v) - ((void*)ps) < (int)sizeof(struct attr_port_stat); v++)
		*v = encode ? htobe64(*v) : be64toh(*v);
	
	return 0;
}


static int qos_endecode (struct attr *at)
{
	struct attr_qos *aq = at->data;
	
	if (at->size != sizeof(struct attr_qos))
		return -EMSGSIZE;
	
	if (aq->port < 1)
		return -EINVAL;
	
	if (aq->prio < PRIO_HIGH || aq->prio > PRIO_LOW)
		return -EINVAL;
	
	return 0;
}


static int bitrate_endecode (struct attr *at, bool encode)
{
	struct attr_bitrate *sb = at->data;
	
	if (at->size != sizeof(struct attr_bitrate))
		return -EMSGSIZE;
	
	if (sb->port < 1)
		return -EINVAL;
	
	if (!encode)
		sb->bitrate = ntohl(sb->bitrate);
	
	if (sb->bitrate < BITRATE_UNSPEC || sb->bitrate > BITRATE_512M)
		return -EINVAL;
	
	if (encode)
		sb->bitrate = htonl(sb->bitrate);
	
	return 0;
}


static int pvid_endecode (struct attr *at, bool encode)
{
	struct attr_pvid *ap = at->data;
	
	if (at->size != sizeof(struct attr_pvid))
		return -EMSGSIZE;
	
	if (ap->port < 1)
		return -EINVAL;
	
	if (!encode)
		ap->vlan = ntohs(ap->vlan);
	
	if (ap->vlan < VLAN_MIN || ap->vlan > VLAN_DOT_MAX)
		return -EINVAL;
	
	if (encode)
		ap->vlan = htons(ap->vlan);
	
	return 0;
}


static int cabletest_do_endecode (struct attr *at)
{
	struct attr_cabletest_do *acd = at->data;
	
	if (at->size != sizeof(struct attr_cabletest_do))
		return -EMSGSIZE;
	
	if (acd->port < 1 || acd->action != 1)
		return -EINVAL;
	
	return 0;
}


static int cabletest_result_endecode (struct attr *at, bool encode)
{
	struct attr_cabletest_result *acr = at->data;
	
	/* Note: this attribute is special
	 * - when sent by the client, it contains the port number whe want to
	 *   get cabletest results from
	 * - when the switch replies, it contains the actual test results data
	 */
	
	/* no need to check the size, the only possible value under 1
	 * is 0 and in that case the decoder is not called
	 */
	if (acr->port < 1)
		return -EINVAL;
	
	if (at->size == 1)
		return 0;
	
	if (at->size != sizeof(struct attr_cabletest_result))
		return -EMSGSIZE;
	
	if (encode) {
		acr->test_result = htonl(acr->test_result);
		acr->fault_distance = htonl(acr->fault_distance);
	} else {
		acr->test_result = ntohl(acr->test_result);
		acr->fault_distance = ntohl(acr->fault_distance);
	}
	
	return 0;
}


static int igmp_vlan_endecode (struct attr *at, bool encode)
{
	struct attr_igmp_vlan *aiv = at->data;
	
	if (at->size != sizeof(struct attr_igmp_vlan))
		return -EMSGSIZE;
	
	if (!encode) {
		aiv->enable = ntohs(aiv->enable);
		aiv->vlan = ntohs(aiv->vlan);
	}
	
	aiv->enable = (aiv->enable != 0);
	if (aiv->vlan < VLAN_MIN || aiv->vlan > VLAN_DOT_MAX)
		return -EINVAL;
	
	if (encode) {
		aiv->enable = htons(aiv->enable);
		aiv->vlan = htons(aiv->vlan);
	}
	
	return 0;
}


static int mirror_encode (struct attr *at)
{
	struct attr_mirror *am = at->data;
	unsigned char p, ports, *r;
	unsigned int sz;
	
	
	/* no need to check attribute size, since
	 * sizeof(struct attr_mirror) == 1 and encoder is not
	 * called when attribute size is zero (ie empty attribute)
	 *
	 * am->outport encodes the outgoing mirror port and the array
	 * the ports from which the data is copied, it must not be empty
	 *
	 * am->outport == 0 is allowed and means mirroring is disabled
	 */
	ports = at->size - sizeof(struct attr_mirror);
	if (am->outport > 0 && ports == 0)
		return -EINVAL;
	
	sz = 3 + ((ports - 1) >> 3);
	r = malloc(sz);
	if (r == NULL)
		return -ENOMEM;
	memset(r, 0, sz);
	
	r[0] = am->outport;
	
	/* FIXME: if ports > 8 */
	for (p = 1; p <= ports; p++) {
		if (am->outport != p)
			r[2] |= (am->ports[p - 1] & 1) << (8 - p);
	}
	
	free(at->data);
	at->data = r;
	at->size = sz;
	
	
	return 0;
}


static int mirror_decode (struct attr *at)
{
	struct attr_mirror *am;
	unsigned char p, ports, *r = at->data;
	unsigned int sz;
	
	
	if (at->size < 3)
		return -EMSGSIZE;
	
	/* note: we cannot compute the exact amount of ports from here,
	 * instead we have the immediate superior multiple of 8
	 * it will be the user's job to ignore extra entries
	 */
	ports = ((at->size - 2) << 3);
	
	/* r[0] == 0 is allowed and means mirroring is disabled */
	if (r[0] == 0)
		ports = 0;
	sz = sizeof(struct attr_mirror) + ports;
	
	am = malloc(sz);
	if (am == NULL)
		return -ENOMEM;
	memset(am, 0, sz);
	
	am->outport = r[0];
	
	/* FIXME: if ports > 8 */
	for (p = 1; p <= ports; p++)
		am->ports[p - 1] = (r[2] >> (8 - p)) & 1;
	
	free(at->data);
	at->data = am;
	at->size = sz;
	
	
	return 0;
}


static int vlan_port_encode (struct attr *at)
{
	struct attr_vlan_conf *avc = at->data;
	unsigned char p, ports, *r;
	unsigned int sz;
	
	
	/* just a header is valid */
	if (at->size < sizeof(struct attr_vlan_conf))
		return -EMSGSIZE;
	ports = at->size - sizeof(struct attr_vlan_conf);
	
	if (avc->vlan < VLAN_MIN || avc->vlan > VLAN_PORT_MAX)
		return -EINVAL;
	
	if (ports == 0)
		sz = 2;
	else
		sz = (2 + 1 + ((ports - 1) >> 3));
	
	r = malloc(sz);
	if (r == NULL)
		return -ENOMEM;
	
	memset(r, 0, sz);
	*(unsigned short*)r = htons(avc->vlan);
	
	/* FIXME: if ports > 8 */
	for (p = 0; p < ports; p++) {
		if (avc->ports[p] == VLAN_UNTAGGED)
			r[2] |= (1 << (7 - p));
	}
	
	free(at->data);
	at->data = r;
	at->size = sz;
	
	
	return 0;
}


static int vlan_port_decode (struct attr *at)
{
	unsigned char p, ports, *r = at->data;
	struct attr_vlan_conf *avc;
	unsigned int sz;
	
	
	if (at->size < 2)
		return -EMSGSIZE;
	
	/* note: we cannot compute the exact amount of ports from here,
	 * instead we have the immediate superior multiple of 8
	 * it will be the user's job to ignore extra entries
	 */
	ports = ((at->size - 2) << 3);
	
	sz = sizeof(struct attr_vlan_conf) + ports;
	avc = malloc(sz);
	if (avc == NULL)
		return -ENOMEM;
	
	avc->vlan = ntohs(*(unsigned short*)r);
	
	/* FIXME: if ports > 8 */
	for (p = 0; p < ports; p++) {
		if ((r[2] >> (7 - p)) & 1)
			avc->ports[p] = VLAN_UNTAGGED;
		else
			avc->ports[p] = VLAN_NO;
	}
	
	free(at->data);
	at->data = avc;
	at->size = sz;
	
	
	return 0;
}


static int vlan_dot_encode (struct attr *at)
{
	struct attr_vlan_conf *avc = at->data;
	unsigned char p, ports, *r, fl;
	unsigned int sz;
	
	
	/* just a header is valid */
	if (at->size < sizeof(struct attr_vlan_conf))
		return -EMSGSIZE;
	ports = at->size - sizeof(struct attr_vlan_conf);
	
	if (avc->vlan < VLAN_MIN || avc->vlan > VLAN_DOT_MAX)
		return -EINVAL;
	
	if (ports == 0)
		sz = 2;
	else
		sz = 2 + 2 * (1 + ((ports - 1) >> 3));
	
	r = malloc(sz);
	if (r == NULL)
		return -EMSGSIZE;
	
	memset(r, 0, sz);
	*(unsigned short*)r = htons(avc->vlan);
	
	/* FIXME: if ports > 8 */
	for (p = 0; p < ports; p++) {
		fl = (1 << (7 - p));
		switch (avc->ports[p]) {
		case VLAN_TAGGED:
			r[3] |= fl;
			/* a tagged VLAN is also marked as untagged
			 * so do not put a "break" here
			 */
		case VLAN_UNTAGGED:
			r[2] |= fl;
		}
	}
	
	
	free(at->data);
	at->data = r;
	at->size = sz;
	
	
	return 0;
}


static int vlan_dot_decode (struct attr *at)
{
	unsigned char p, ports, *r = at->data;
	struct attr_vlan_conf *avc;
	unsigned int sz;
	
	
	/* attribute size must be a multiple of 2 because there are
	 * 2 bytes (1 for tagged and 1 for untagged) per block of
	 * 8 ports, plus the 2 first bytes for the VLAN id
	 */
	if (at->size < 2 || (at->size & 1) != 0)
		return -EMSGSIZE;
	
	/* note: we cannot compute the exact amount of ports from here,
	 * instead we have the immediate superior multiple of 8
	 * it will be the user's job to ignore extra entries
	 */
	ports = ((at->size - 2) / 2) << 3;
	
	sz = sizeof(struct attr_vlan_conf) + ports;
	avc = malloc(sz);
	if (avc == NULL)
		return -ENOMEM;
	
	avc->vlan = ntohs(*(unsigned short*)r);
	
	/* FIXME: if ports > 8 */
	for (p = 0; p < ports; p++) {
		if ((r[3] >> (7 - p)) & 1)
			avc->ports[p] = VLAN_TAGGED;
		else if ((r[2] >> (7 - p)) & 1)
			avc->ports[p] = VLAN_UNTAGGED;
		else
			avc->ports[p] = VLAN_NO;
	}
	
	free(at->data);
	at->data = avc;
	at->size = sz;
	
	
	return 0;
}


static int processAttr (struct attr *at, bool encode)
{
	unsigned char *byte = at->data;
	unsigned short *word = at->data;
	unsigned int *dword = at->data;
	
	
	/* empty attributes are not processed */
	if (at->size == 0)
		return 0;
	
	if (at->data == NULL)
		return -EFAULT;
	
	switch (at->attr) {
	
	case ATTR_MAC:
		if (at->size != ETH_ALEN)
			return -EMSGSIZE;
		return 0;
	
	case ATTR_IP:
	case ATTR_NETMASK:
	case ATTR_GATEWAY:
		/* IP addresses are kept in network byte order even on the host */
		if (at->size != sizeof(struct in_addr))
			return -EMSGSIZE;
		
		return 0;
	
	case ATTR_PORTS_COUNT:
		if (at->size != 1)
			return -EMSGSIZE;
		
		return 0;
	
	case ATTR_RESTART:
	case ATTR_DEFAULTS:
	case ATTR_STATS_RESET:
	case ATTR_STORM_ENABLE:
	case ATTR_IGMP_BLOCK_UNK:
	case ATTR_IGMP_VALID_V3:
	case ATTR_LOOP_DETECT:
		if (at->size != 1)
			return -EMSGSIZE;
		
		*byte = (*byte != 0);
		
		return 0;
	
	case ATTR_DHCP:
		/* Note: DHCP attribute is special, it is 2 two bytes long
		 * when sent by the switch but only 1 byte long when sent
		 * by the client
		 */
		if (at->size == 1) {
			*byte = (*byte != 0);
			return 0;
		} else if (at->size > 2) {
			return -EMSGSIZE;
		}
		
		if (!encode)
			*word = ntohs(*word);
		
		*word = (*word != 0);
		
		if (encode)
			*word = htons(*word);
		
		return 0;
	
	case ATTR_ENCPASS:
		if (at->size != 4)
			return -EMSGSIZE;
		
		if (!encode)
			*dword = ntohl(*dword);
		
		*dword = (*dword != 0);
		
		if (encode)
			*dword = htonl(*dword);
		
		return 0;
	
	case ATTR_VLAN_TYPE:
		if (at->size != 1)
			return -EMSGSIZE;
		
		/* no need to check if *byte < VLAN_DISABLED because
		 * byte is unsigned and VLAN_DISABLED is 0 */
		if (*byte > VLAN_DOT_ADV)
			return -EINVAL;
		
		return 0;
	
	case ATTR_QOS_TYPE:
		if (at->size != 1)
			return -EMSGSIZE;
		
		if (*byte < QOS_PORT || *byte > QOS_DOT)
			return -EINVAL;
		
		return 0;
	
	case ATTR_QOS_CONFIG:
		return qos_endecode(at);
	
	case ATTR_VLAN_DESTROY:
		if (at->size != 2)
			return -EMSGSIZE;
		
		if (!encode)
			*word = ntohs(*word);
		
		if (*word < VLAN_MIN || *word > VLAN_DOT_MAX)
			return -EINVAL;
		
		if (encode)
			*word = htons(*word);
		
		return 0;
	
	case ATTR_PORT_STATUS:
		return ports_status_endecode(at);
	
	case ATTR_PORT_STATISTICS:
		return port_stat_endecode(at, encode);
	
	case ATTR_BITRATE_INPUT:
	case ATTR_BITRATE_OUTPUT:
	case ATTR_STORM_BITRATE:
		return bitrate_endecode(at, encode);
	
	case ATTR_VLAN_PVID:
		return pvid_endecode(at, encode);
	
	case ATTR_CABLETEST_DO:
		return cabletest_do_endecode(at);
	
	case ATTR_CABLETEST_RESULT:
		return cabletest_result_endecode(at, encode);
	
	case ATTR_IGMP_ENABLE_VLAN:
		return igmp_vlan_endecode(at, encode);
	
	case ATTR_MIRROR:
		if (encode)
			return mirror_encode(at);
		else
			return mirror_decode(at);
	
	case ATTR_VLAN_PORT_CONF:
		if (encode)
			return vlan_port_encode(at);
		else
			return vlan_port_decode(at);
	
	case ATTR_VLAN_DOT_CONF:
		if (encode)
			return vlan_dot_encode(at);
		else
			return vlan_dot_decode(at);
	
	/* undefined attributes are not modified */
	default:
		return 0;
	}
}


int encodeAttr (struct attr *at)
{
	return processAttr(at, true);
}


int decodeAttr (struct attr *at)
{
	return processAttr(at, false);
}


