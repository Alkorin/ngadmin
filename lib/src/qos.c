
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


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


