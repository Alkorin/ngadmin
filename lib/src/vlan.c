
#include <ngadmin.h>

#include <attr.h>
#include <protocol.h>

#include "lib.h"
#include "network.h"


int ngadmin_getVLANType (struct ngadmin *nga, int *t)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	
	
	if (nga == NULL || t == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_VLAN_TYPE));
	ret=readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_TYPE, ATTR_END);
	
	*t = VLAN_DISABLED;
	
	if (attr->first != NULL) {
		at = attr->first->data;
		*t =(int)*(char*)at->data;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setVLANType (struct ngadmin *nga, int t)
{
	List *attr;
	
	
	if (nga == NULL || t < 1 || t > 4)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_VLAN_TYPE, t));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getVLANPortConf (struct ngadmin *nga, unsigned char *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_vlan_conf *avc;
	struct swi_attr *sa;
	int port;
	
	
	if (nga == NULL || ports== NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_VLAN_PORT_CONF));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_PORT_CONF, ATTR_END);
	
	memset(ports, 0, sa->ports);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		avc = at->data;

		if (at->size != sizeof(struct attr_vlan_conf) + sa->ports)
			return ERR_INVARG;
		
		for (port = 0; port < sa->ports; port++) {
			if (avc->ports[port] == VLAN_UNTAGGED)
				ports[port] = avc->vlan;
		}
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setVLANPortConf (struct ngadmin *nga, const unsigned char *ports)
{
	List *attr = NULL;
	ListNode *ln;
	struct attr *at;
	struct swi_attr *sa;
	struct attr_vlan_conf *avc;
	int ret = ERR_OK, port;
	unsigned char vlan;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	/* if nothing is to be changed, do nothing */
	for (port = 0; port < sa->ports && ports[port] == 0; port++);
	if (port == sa->ports )
		goto end;
	
	attr = createEmptyList();

	if (memchr(ports, 0, sa->ports) != NULL) {
		/* if at least one port is unchanged, we need to read old config */
		pushBackList(attr, newEmptyAttr(ATTR_VLAN_PORT_CONF));
		ret = readRequest(nga, attr);
		if (ret != ERR_OK)
			goto end;
		
		filterAttributes(attr, ATTR_VLAN_PORT_CONF, ATTR_END);
		/* FIXME: check if the returned array effectively contains correct data */
	} else {
		/* create an empty VLAN config */
		for (vlan = VLAN_MIN; vlan <= VLAN_PORT_MAX; vlan++) {
			avc = malloc(sizeof(struct attr_vlan_conf) + sa->ports);
			avc->vlan = vlan;
			memset(avc->ports, 0, sa->ports);
			pushBackList(attr, newAttr(ATTR_VLAN_PORT_CONF, sizeof(struct attr_vlan_conf) + sa->ports, avc));
		}
	}
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		avc = at->data;
		for (port = 0; port < sa->ports; port++) {
			if (ports[port] == avc->vlan)
				avc->ports[port] = VLAN_UNTAGGED;
			else
				avc->ports[port] = VLAN_NO;
		}
	}
	
	ret = writeRequest(nga, attr);
	attr = NULL;
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_getVLANDotAllConf (struct ngadmin *nga, unsigned short *vlans, unsigned char *ports, int *nb)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK, total;
	struct attr_vlan_conf *avc;
	struct swi_attr *sa;
	
	
	if (nga == NULL || vlans == NULL || ports== NULL || nb == NULL || *nb <= 0)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	total = *nb;
	*nb = 0;
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_VLAN_DOT_CONF));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_DOT_CONF, ATTR_END);
	
	memset(vlans, 0, total * sizeof(unsigned short));
	memset(ports, 0, total * sa->ports);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		avc = at->data;
		
		if (at->size != sizeof(struct attr_vlan_conf) + sa->ports)
			return ERR_INVARG;
		
		*vlans = avc->vlan;
		memcpy(ports, avc->ports, sa->ports);
		
		vlans++;
		ports += sa->ports;
		(*nb)++;
		
		if (*nb > total)
			break; /* no more room */
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_getVLANDotConf (struct ngadmin *nga, unsigned short vlan, unsigned char *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_vlan_conf *avc;
	
	
	if (nga == NULL || vlan < VLAN_MIN || vlan > VLAN_DOT_MAX || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newShortAttr(ATTR_VLAN_DOT_CONF, vlan));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_DOT_CONF, ATTR_END);
	
	memset(ports, 0, nga->current->ports);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		avc = at->data;
		if (avc->vlan == vlan) {
			memcpy(ports, avc->ports, nga->current->ports);
			break;
		}
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setVLANDotConf (struct ngadmin *nga, unsigned short vlan, const unsigned char *ports)
{
	List *attr = NULL;
	struct attr *at;
	struct swi_attr *sa;
	struct attr_vlan_conf *avc;
	int ret = ERR_OK, port;
	
	
	if (nga == NULL || vlan < VLAN_MIN || vlan > VLAN_DOT_MAX || ports == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	
	/* if nothing is to be changed, do nothing */
	for (port = 0; port < sa->ports && ports[port] == VLAN_UNSPEC; port++);
	if (port == sa->ports )
		goto end;
	
	
	attr = createEmptyList();
	avc = malloc(sizeof(struct attr_vlan_conf) + sa->ports);
	if (avc == NULL)
		return ERR_MEM;
	
	avc->vlan = vlan;
	
	/* if all is to be changed, we do not need to read old config */
	if (memchr(ports, VLAN_UNSPEC, sa->ports) != NULL) {
		
		pushBackList(attr, newShortAttr(ATTR_VLAN_DOT_CONF, vlan));
		ret = readRequest(nga, attr);
		if (ret != ERR_OK)
			goto end;
		
		filterAttributes(attr, ATTR_VLAN_DOT_CONF, ATTR_END);
		
		if (attr->first != NULL) {
			at = attr->first->data;
			memcpy(avc, at->data, sizeof(struct attr_vlan_conf) + sa->ports);
		}
		
		clearList(attr, (void(*)(void*))freeAttr);
	}
	
	
	/* apply changes */
	for (port = 0; port < sa->ports; port++) {
		if (ports[port] != VLAN_UNSPEC)
			avc->ports[port] = ports[port];
	}
	
	
	pushBackList(attr, newAttr(ATTR_VLAN_DOT_CONF, sizeof(struct attr_vlan_conf) + sa->ports, avc));
	ret = writeRequest(nga, attr);
	attr = NULL;
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_VLANDestroy (struct ngadmin *nga, unsigned short vlan)
{
	List *attr;
	
	
	if (nga == NULL || vlan < VLAN_MIN || vlan > VLAN_DOT_MAX)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newShortAttr(ATTR_VLAN_DESTROY, vlan));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_getAllPVID (struct ngadmin *nga, unsigned short *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_pvid *ap;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_VLAN_PVID));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_VLAN_PVID, ATTR_END);
	
	memset(ports, 0, nga->current->ports * sizeof(unsigned short));
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		ap = at->data;
		ports[ap->port - 1] = ap->vlan;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setPVID (struct ngadmin *nga, unsigned char port, unsigned short vlan)
{
	List *attr;
	struct attr_pvid *ap;
	
	
	if (nga == NULL || port < 1 || vlan < VLAN_MIN || vlan > VLAN_DOT_MAX)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	else if (port > nga->current->ports)
		return ERR_INVARG;
	
	
	attr = createEmptyList();
	ap = malloc(sizeof(struct attr_pvid));
	if (ap == NULL)
		return ERR_MEM;
	ap->port = port;
	ap->vlan = vlan;
	
	pushBackList(attr, newAttr(ATTR_VLAN_PVID, sizeof(struct attr_pvid), ap));
	
	
	return writeRequest(nga, attr);
}

