
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


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
	struct swi_attr *sa;
	
	
	if (nga == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	p = malloc(1 + sa->ports);
	if (p == NULL)
		return ERR_MEM;
	
	if (ports == NULL)
		memset(p, 0, 1 + sa->ports);
	else
		memcpy(p, ports, 1 + sa->ports);
	
	attr = createEmptyList();
	pushBackList(attr, newAttr(ATTR_MIRROR, 1 + sa->ports, p));
	
	
	return writeRequest(nga, attr);
}

