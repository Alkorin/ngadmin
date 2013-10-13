
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
	struct swi_attr *sa;
	struct attr_mirror *am;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_MIRROR));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_MIRROR, ATTR_END);
	
	memset(ports, 0, 1 + sa->ports);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		am = at->data;
		
		if (am->outport == 0) {
			memset(ports, 0, 1 + sa->ports);
		} else if (am->outport > 0 && at->size >= 1 + sa->ports) {
			if (at->size < sizeof(struct attr_mirror) + sa->ports) {
				ret = ERR_INVARG;
				goto end;
			}
			ports[0] = am->outport;
			memcpy(ports + 1, am->ports, sa->ports);
		}
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setMirror (struct ngadmin *nga, const char *ports)
{
	List *attr;
	struct swi_attr *sa;
	struct attr_mirror *am;
	
	
	if (nga == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	am = malloc(sizeof(struct attr_mirror) + sa->ports);
	if (am == NULL)
		return ERR_MEM;
	
	if (ports == NULL) {
		am->outport = 0;
	} else {
		am->outport = ports[0];
		memcpy(am->ports, ports + 1, sa->ports);
	}
	
	attr = createEmptyList();
	pushBackList(attr, newAttr(ATTR_MIRROR, sizeof(struct attr_mirror) + sa->ports, am));
	
	
	return writeRequest(nga, attr);
}

