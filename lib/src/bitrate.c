
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


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


