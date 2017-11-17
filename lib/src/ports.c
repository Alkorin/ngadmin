
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


int ngadmin_getPortsStatus (struct ngadmin *nga, unsigned char *ports)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_port_status *ps;
	struct swi_attr *sa;
	
	
	if (nga == NULL || ports == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_PORT_STATUS));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_PORT_STATUS, ATTR_END);
	
	memset(ports, SPEED_UNK, sa->ports);
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		ps = at->data;
		if (at->size == 0) {
			ret = ERR_BADREPLY;
			goto end;
		}
		if (ps->port <= sa->ports)
			ports[ps->port - 1] = ps->status;
	}
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_getPortsStatistics (struct ngadmin *nga, struct port_stats *ps)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK;
	struct attr_port_stat *aps;
	struct swi_attr *sa;
	
	
	if (nga == NULL || ps == NULL)
		return ERR_INVARG;
	
	sa = nga->current;
	if (sa == NULL)
		return ERR_NOTLOG;
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_PORT_STATISTICS));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_PORT_STATISTICS, ATTR_END);
	
	memset(ps, 0, sa->ports * sizeof(struct port_stats));
	
	for (ln = attr->first; ln != NULL; ln = ln->next) {
		at = ln->data;
		aps = at->data;
		if (at->size == 0) {
			ret = ERR_BADREPLY;
			goto end;
		}
		if (aps->port <= sa->ports) {
			ps[aps->port -1].recv = aps->recv;
			ps[aps->port -1].sent = aps->sent;
			ps[aps->port -1].crc = aps->crc;
		}
	}
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_resetPortsStatistics (struct ngadmin *nga)
{
	List *attr;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_STATS_RESET, 1));
	
	
	return writeRequest(nga, attr);
}


int ngadmin_cabletest (struct ngadmin *nga, struct cabletest *ct, int nb)
{
	List *attr;
	ListNode *ln;
	struct attr *at;
	int ret = ERR_OK, i;
	struct attr_cabletest_do *acd;
	struct attr_cabletest_result *acr;
	
	
	if (nga == NULL || ct == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	
	attr = createEmptyList();
	
	for (i = 0; i < nb; i++) {
		
		acd = malloc(sizeof(struct attr_cabletest_do));
		if (acd == NULL)
			return ERR_MEM;
		acd->port = ct[i].port;
		acd->action = 1;
		pushBackList(attr, newAttr(ATTR_CABLETEST_DO, sizeof(struct attr_cabletest_do), acd));
		
		ret = writeRequest(nga, attr);
		attr = NULL;
		if (ret < 0)
			goto end;
		
		/* the list is destroyed by writeRequest, so we need to recreate it */
		attr = createEmptyList();
		pushBackList(attr, newByteAttr(ATTR_CABLETEST_RESULT, ct[i].port));
		ret = readRequest(nga, attr);
		if (ret < 0)
			goto end;
		
		filterAttributes(attr, ATTR_CABLETEST_RESULT, ATTR_END);
		
		for (ln = attr->first; ln != NULL; ln = ln->next) {
			at = ln->data;
			acr = at->data;
			if (at->size != sizeof(struct attr_cabletest_result)) {
				ret = ERR_BADREPLY;
				goto end;
			}
			if (acr->port == ct[i].port) {
				ct[i].test_result = acr->test_result;
				ct[i].fault_distance = acr->fault_distance;
				break;
			}
		}
		
		/* just empty the list, it will be used at next iteration */
		clearList(attr, (void(*)(void*))freeAttr);
	}
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_getLoopDetectionState (struct ngadmin *nga, int *s)
{
	List *attr;
	struct attr *at;
	int ret = ERR_OK;
	
	
	if (nga == NULL || s == NULL)
		return ERR_INVARG;
	else if (nga->current == NULL)
		return ERR_NOTLOG;
	
	attr = createEmptyList();
	pushBackList(attr, newEmptyAttr(ATTR_LOOP_DETECT));
	ret = readRequest(nga, attr);
	if (ret != ERR_OK)
		goto end;
	
	filterAttributes(attr, ATTR_LOOP_DETECT, ATTR_END);
	
	*s = 0;
	
	if (attr->first == NULL) {
		ret = ERR_BADREPLY;
		goto end;
	}
	at = attr->first->data;
	if (at->size != 1) {
		ret = ERR_BADREPLY;
		goto end;
	}
	*s = *(char *)at->data;
	
	
end:
	destroyList(attr, (void(*)(void*))freeAttr);
	
	
	return ret;
}


int ngadmin_setLoopDetectionState (struct ngadmin *nga, int s)
{
	List *attr;
	
	
	attr = createEmptyList();
	pushBackList(attr, newByteAttr(ATTR_LOOP_DETECT, s != 0));
	
	
	return writeRequest(nga, attr);
}


