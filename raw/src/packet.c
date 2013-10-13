
#include <string.h>
#include <errno.h>

#include <nsdp/attr.h>
#include <nsdp/packet.h>


void initNsdpHeader (struct nsdp_header *nh, const struct nsdp_cmd *nc)
{
	memset(nh, 0, sizeof(struct nsdp_header));
	nh->version = NSDP_VERSION;
	nh->code = nc->code;
	nh->error = nc->error;
	nh->attr = htons(nc->attr_error);
	
	memcpy(nh->client_mac, &nc->client_mac, ETH_ALEN);
	memcpy(nh->switch_mac, &nc->switch_mac, ETH_ALEN);
	
	nh->seqnum = htonl(nc->seqnum);
	memcpy(nh->proto_id, NSDP_PROTOID, 4);
}


bool extractNsdpHeader (const struct nsdp_header *nh, struct nsdp_cmd *nc)
{
	unsigned int i;
	
	
	if (nh->version != NSDP_VERSION)
		return false;
	
	if (nc->code > 0 && nh->code != nc->code)
		return false;
	nc->code = nh->code;
	
	nc->error = nh->error;
	nc->attr_error = ntohs(nh->attr);
	
	if (nh->unk1 != 0)
		return false;
	
	if (*(unsigned short*)nh->unk2 != 0)
		return false;
	
	for (i = 0; i < ETH_ALEN && nc->client_mac.ether_addr_octet[i] == 0; i++);
	if (i < ETH_ALEN && memcmp(nh->client_mac, &nc->client_mac, ETH_ALEN) != 0)
		return false;
	memcpy(&nc->client_mac, nh->client_mac, ETH_ALEN);
	
	for (i = 0; i < ETH_ALEN && nc->switch_mac.ether_addr_octet[i] == 0; i++);
	if (i < ETH_ALEN && memcmp(nh->switch_mac, &nc->switch_mac, ETH_ALEN) != 0)
		return false;
	memcpy(&nc->switch_mac, nh->switch_mac, ETH_ALEN);
	
	if (nc->seqnum > 0 && ntohl(nh->seqnum) != nc->seqnum)
		return false;
	nc->seqnum = ntohl(nh->seqnum);
	
	if (memcmp(nh->proto_id, NSDP_PROTOID, 4) != 0)
		return false;
	
	if (*(unsigned int*)nh->unk3 != 0)
		return false;
	
	return true;
}


int addPacketAttributes (struct nsdp_packet *np, const List* attr)
{
	ListNode *ln;
	struct attr *at;
	
	
	if (attr == NULL)
		return 0;
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		
		/* encode attribute data */
		if (encodeAttr(at) < 0)
			return -EINVAL;
		
		/* attribute data bigger than the remaining size: error */
		if ((int)(getPacketTotalSize(np) + sizeof(struct attr_header) + at->size) > np->maxlen)
			return -EMSGSIZE;
		
		/* set attribute code and size */
		np->ah->attr = htons(at->attr);
		np->ah->size = htons(at->size);
		
		/* copy attribute data */
		if (at->size > 0 && at->data != NULL)
			memcpy(np->ah->data, at->data, at->size);
		
		/* move to next attribute */
		np->ah = (struct attr_header*)(np->ah->data + at->size);
	}
	
	
	return 0;
}


int extractPacketAttributes (struct nsdp_packet *np, List *attr)
{
	struct attr *at;
	unsigned short size;
	
	
	while (getPacketTotalSize(np) < np->maxlen) {
		/* no room for an attribute header: error */
		if (getPacketTotalSize(np) + (int)sizeof(struct attr_header) > np->maxlen)
			return -EMSGSIZE;
		
		/* create new attribute */
		size = ntohs(np->ah->size);
		at = newAttr(ntohs(np->ah->attr), size, NULL);
		if (at == NULL)
			return -ENOMEM;
		
		/* attribute data bigger than the remaining size: error */
		if (getPacketTotalSize(np) + (int)sizeof(struct attr_header) + size > np->maxlen) {
			free(at);
			return -EMSGSIZE;
		}
		
		/* copy attribute raw data */
		if (size == 0) {
			at->data = NULL;
		} else {
			at->data = malloc(size * sizeof(unsigned char));
			memcpy(at->data, np->ah->data, size);
		}
		
		/* decode attribute data */
		if(decodeAttr(at) == 0) {
			/* stop on an END attribute */
			if (at->attr == ATTR_END) {
				free(at);
				break;
			} else {
				pushBackList(attr, at);
			}
		} else {
			free(at);
		}
		
		/* move to next attribute */
		np->ah = (struct attr_header*)(np->ah->data + size);
	}
	
	
	return 0;
}


