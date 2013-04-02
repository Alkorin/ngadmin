
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "attr.h"
#include "protocol.h"



int trim (char *txt, int start)
{
	char *p;
	
	if (txt == NULL)
		return 0;
	
	p = txt + start;
	while (p >= txt && (*p == ' ' || *p == '\n')) {
		*p = '\0';
		p--;
	}
	
	return p - txt + 1;
}


void initNgHeader (struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum)
{
	memset(nh, 0, sizeof(struct ng_header));
	nh->version = 1;
	nh->code = code;
	
	memcpy(nh->client_mac, client_mac, ETH_ALEN);
	
	if (switch_mac != NULL)
		memcpy(nh->switch_mac, switch_mac, ETH_ALEN);
	
	nh->seqnum = htonl(seqnum);
	memcpy(nh->proto_id, "NSDP", 4);
}


bool validateNgHeader (const struct ng_header *nh, char code, const struct ether_addr *client_mac, const struct ether_addr *switch_mac, unsigned int seqnum)
{
	if (nh->version != 1)
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
	
	if (memcmp(nh->proto_id, "NSDP", 4) != 0)
		return false;
	
	if (*(unsigned int*)nh->unk3 != 0)
		return false;
	
	return true;
}


void addPacketAttr (struct ng_packet *np, struct attr *at)
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


int addPacketAttributes (struct ng_packet *np, const List* attr, unsigned char ports)
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


int extractPacketAttributes (struct ng_packet *np, List *attr, unsigned char ports)
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
			at->data = malloc(size * sizeof(char));
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


void filterAttributes (List *attr, ...)
{
	va_list ap;
	ListNode *ln, *pr;
	struct attr *at;
	unsigned short attrcode;
	bool keep;
	
	
	ln = attr->first;
	while (ln != NULL) {
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


