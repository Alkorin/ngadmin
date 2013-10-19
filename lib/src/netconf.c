
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


int ngadmin_getIGMPConf (struct ngadmin *nga, struct igmp_conf *ic)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_igmp_vlan *aiv;
	
	
	if (nga == NULL || ic == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	/* ATTR_IGMP_ENABLE_VLAN
	 * ATTR_IGMP_BLOCK_UNK
	 * ATTR_IGMP_VALID_V3
	 *
	 * apparently, read-querying these attributes at the same time causes
	 * the switch to reply garbage
	 * here we are forced to do like the official win app and send a
	 * separate request for each attribute
	 */
	
	attr = createEmptyList();
	memset(ic, 0, sizeof(struct igmp_conf));
	
	
	pushBackList(attr, newEmptyAttr(ATTR_IGMP_ENABLE_VLAN));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_IGMP_ENABLE_VLAN, ATTR_END);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		aiv = at->data;
		ic->enable = aiv->enable;
		ic->vlan = aiv->vlan;
	}
	
	clearList(attr, (void(*)(void*))freeAttr);
	
	
	pushBackList(attr, newEmptyAttr(ATTR_IGMP_BLOCK_UNK));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_IGMP_BLOCK_UNK, ATTR_END);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		ic->block = *(char*)at->data;
	}
	
	clearList(attr, (void(*)(void*))freeAttr);
	
	
	pushBackList(attr, newEmptyAttr(ATTR_IGMP_VALID_V3));
	ret = readRequest(nga, attr);
	if (ret < 0)
		goto end;
	
	filterAttributes(attr, ATTR_IGMP_VALID_V3, ATTR_END);
	
	if (attr->first != NULL) {
		at = attr->first->data;
		ic->validate = *(char*)at->data;
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setIGMPConf (struct ngadmin *nga, const struct igmp_conf *ic)
{
	List *attr;
	struct attr_igmp_vlan *aiv;
	
	
	if (nga == NULL || ic == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	aiv = malloc(sizeof(struct attr_igmp_vlan));
	if (aiv == NULL)
		return ERR_MEM;
	aiv->enable = ic->enable;
	aiv->vlan = ic->vlan;
	
	
	attr = createEmptyList();
	pushBackList(attr, newAttr(ATTR_IGMP_ENABLE_VLAN, sizeof(struct attr_igmp_vlan), aiv));
	pushBackList(attr, newByteAttr(ATTR_IGMP_BLOCK_UNK, ic->block != false));
	pushBackList(attr, newByteAttr(ATTR_IGMP_VALID_V3, ic->validate != false));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_setNetConf (struct ngadmin *nga, const struct net_conf *nc)
{
	List *attr;
	int ret = ERR_OK;
	struct swi_attr *sa;
	struct net_conf nc_new;
	
	
	if (nga == NULL || nc == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	memcpy(&nc_new, &sa->nc, sizeof(struct net_conf));
	if (nc->ip.s_addr != 0)
		nc_new.ip = nc->ip;
	if (nc->netmask.s_addr != 0)
		nc_new.netmask = nc->netmask;
	if (nc->gw.s_addr != 0)
		nc_new.gw = nc->gw;
	nc_new.dhcp = nc->dhcp;
	
	/* gateway must be in the network range */
	if ((nc_new.ip.s_addr & nc_new.netmask.s_addr) != (nc_new.gw.s_addr & nc_new.netmask.s_addr))
		return ERR_INVARG;
	
	/* no need to send anything if old and new configurations are the same */
	if (memcmp(&nc_new, &sa->nc, sizeof(struct net_conf)) == 0)
		return ERR_OK;
	
	
	attr = createEmptyList();
	
	/* Note: DHCP attribute is special, it is 2 two bytes long when sent
	 * by the switch but only 1 byte long when sent by the client
	 */
	if (nc_new.dhcp) {
		pushBackList(attr, newByteAttr(ATTR_DHCP, 1));
	} else {
		pushBackList(attr, newAddrAttr(ATTR_IP, nc_new.ip));
		pushBackList(attr, newAddrAttr(ATTR_NETMASK, nc_new.netmask));
		pushBackList(attr, newAddrAttr(ATTR_GATEWAY, nc_new.gw));
		pushBackList(attr, newByteAttr(ATTR_DHCP, 0));
	}
	
	ret = writeRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	
	/* update local values */
	memcpy(&sa->nc, &nc_new, sizeof(struct net_conf));
	
end:
	
	return ret;
}


