
#include <string.h>
#include <errno.h>

#include <attr.h>
#include <encoding.h>
#include "encoding_attr.h"



void initNsdpHeader (struct nsdp_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum)
{
	memset(nh, 0, sizeof(struct nsdp_header));
	nh->version = NSDP_VERSION;
	nh->code = code;
	
	memcpy(nh->client_mac, client_mac, ETH_ALEN);
	
	if (switch_mac != NULL)
		memcpy(nh->switch_mac, switch_mac, ETH_ALEN);
	
	nh->seqnum = htonl(seqnum);
	memcpy(nh->proto_id, NSDP_PROTOID, 4);
}


bool validateNsdpHeader (const struct nsdp_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum)
{
	if (nh->version != NSDP_VERSION)
		return false;
	
	if (code > 0 && nh->code != code)
		return false;
	
	if (nh->unk1 != 0)
		return false;
	
	if (*(unsigned short*)nh->unk2 != 0)
		return false;
	
	if (client_mac != NULL && memcmp(nh->client_mac, client_mac, ETH_ALEN) != 0)
		return false;
	
	if (switch_mac != NULL && memcmp(nh->switch_mac, switch_mac, ETH_ALEN) != 0)
		return false;
	
	if (seqnum > 0 && ntohl(nh->seqnum) != seqnum)
		return false;
	
	if (memcmp(nh->proto_id, NSDP_PROTOID, 4) != 0)
		return false;
	
	if (*(unsigned int*)nh->unk3 != 0)
		return false;
	
	return true;
}


static void addPacketAttr (struct nsdp_packet *np, struct attr *at)
{
	struct attr_header *ah = np->ah;
	
	
	if ((int)(getPacketTotalSize(np) + sizeof(struct attr_header) + at->size) > np->maxlen)
		return;
	
	ah->attr = htons(at->attr);
	ah->size = htons(at->size);
	
	if (at->size > 0 && at->data != NULL)
		memcpy(ah->data, at->data, at->size);
	
	np->ah = (struct attr_header*)(ah->data + at->size);
}


int addPacketAttributes (struct nsdp_packet *np, const List* attr, unsigned char ports)
{
	ListNode *ln;
	struct attr *at;
	const struct attr_handler *ah;
	
	
	if (attr == NULL)
		return 0;
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		ah = getAttrHandler(at->attr);
		if (ah != NULL) {
			if (ah->size > 0 && at->size > 0 && at->size != ah->size)
				return -EINVAL;
			if (at->size > 0 && ah->encode != NULL && !ah->encode(at, ports))
				return -EINVAL;
		}
		
		addPacketAttr(np, at);
	}
	
	
	return 0;
}


int extractPacketAttributes (struct nsdp_packet *np, List *attr, unsigned char ports)
{
	struct attr *at;
	const struct attr_handler *ah;
	int ret = 0;
	unsigned short size;
	bool valid;
	
	
	while (getPacketTotalSize(np) < np->maxlen) {
		/* no room for an attribute header: error */
		if (getPacketTotalSize(np) + (int)sizeof(struct attr_header) > np->maxlen) {
			ret = -1;
			break;
		}
		
		/* create new attribute */
		size = ntohs(np->ah->size);
		at = newAttr(ntohs(np->ah->attr), size, NULL);
		if (at == NULL) {
			ret = -1;
			break;
		}
		
		/* attribute data bigger than the remaining size: error */
		if (getPacketTotalSize(np) + (int)sizeof(struct attr_header) + size > np->maxlen) {
			free(at);
			ret = -1;
			break;
		}
		
		/* copy attribute raw data */
		if (size == 0) {
			at->data = NULL;
		} else {
			at->data = malloc(size * sizeof(unsigned char));
			memcpy(at->data, np->ah->data, size);
		}
		
		/* decode attribute data */
		valid = true;
		
		ah = getAttrHandler(at->attr);
		if (at->data == NULL || ah == NULL)
			goto next;
		
		if (ah->size > 0 && size != ah->size) {
			valid = false;
			goto next;
		}
		
		if (ah->decode != NULL)
			valid = ah->decode(at, ports);
		
next:
		/* stop on an END attribute */
		if (at->attr == ATTR_END) {
			free(at);
			break;
		}
		
		/* move to next attribute */
		np->ah = (struct attr_header*)(np->ah->data + size);
		
		if (valid)
			pushBackList(attr, at);
		else
			free(at);
	}
	
	
	return ret;
}


