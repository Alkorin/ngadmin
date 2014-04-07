
#include <errno.h>

#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


int ngadmin_scan (struct ngadmin *nga)
{
	int i;
	List *attr, *swiList;
	struct swi_attr *sa;
	struct nsdp_cmd nc;
	/* sent by official win client:
	 * ATTR_PRODUCT
	 * ATTR_UNK2
	 * ATTR_NAME
	 * ATTR_MAC
	 * ATTR_UNK5
	 * ATTR_IP
	 * ATTR_NETMASK
	 * ATTR_GATEWAY
	 * ATTR_DHCP
	 * ATTR_UNK12
	 * ATTR_FIRM_VER
	 * ATTR_UNK14
	 * ATTR_UNK15
	 * ATTR_END
	 *
	 * one may be tempted to add ATTR_ENCPASS so we can now early if the
	 * switch uses password encryption, but this would cause (at least)
	 * switches that do not support this feature not to reply to the
	 * discovery request at all
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
	prepareSend(nga, &nc, CODE_READ_REQ);
	i = sendNsdpPacket(nga->sock, &nc, attr);
	clearList(attr, (void(*)(void*))freeAttr);
	if (i == -EINVAL)
		return ERR_INVARG;
	else if (i < 0)
		return ERR_NET;
	
	/* try to receive any packets until timeout */
	swiList = createEmptyList();
	/* FIXME: end after timeout whatever received packet is good or not */
	while (1) {
		prepareRecv(nga, &nc, CODE_READ_REP);
		if (recvNsdpPacket(nga->sock, &nc, attr, &nga->timeout) < 0)
			break;
		
		sa = malloc(sizeof(struct swi_attr));
		if (sa == NULL)
			return ERR_MEM;
		
		if (extractSwitchAttributes(sa, attr) == 0)
			pushBackList(swiList, sa);
		
		clearList(attr, (void(*)(void*))freeAttr);
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
	struct attr *at;
	int ret = ERR_OK;
	struct swi_attr *sa;
	
	
	if (nga == NULL)
		return ERR_INVARG;
	else if (id < 0 || id >= nga->swi_count)
		return ERR_BADID;
	
	sa = &nga->swi_tab[id];
	nga->current = sa;
	nga->encrypt_pass = false;
	
	/* determine if the switch uses password encryption
	 * as explained in ngadmin_scan, it cannot be done at discovery
	 * stage
	 */
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_ENCPASS));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_ENCPASS, ATTR_END);
	if (attr->first != NULL) {
		at = attr->first->data;
		nga->encrypt_pass = (at->size == 4 && *(unsigned int*)at->data == 1);
	}
	clearList(attr, (void(*)(void*))freeAttr);
	
	/* strangely, passwords must never be encrypted inside a read request,
	 * or it will be rejected. Seems more to be a firmware bug
	 */
	pushBackList(attr, newAttr(ATTR_PASSWORD, strlen(nga->password), strdup(nga->password)));
	ret = readRequest(nga, attr);
	
	if (ret == ERR_INVOP) {
		/* it seems some switches do not support login with read request
		 * fallback to write request, even if it has the drawback of
		 * the password being broadcasted back by the switch
		 */
		ret = writeRequest(nga, NULL);
	}
	
	if (ret == ERR_OK ) {
		/* login succeeded */
		/* TODO: if keep broadcasting is disabled, connect() the UDP
		 * socket so icmp errors messages (port unreachable, TTL exceeded
		 * in transit, ...) can be received
		 */
	} else {
		/* login failed */
		nga->current = NULL;
	}
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


