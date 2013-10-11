
#include <ngadmin.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>

#include "lib.h"
#include "network.h"


static const struct timeval default_timeout = {.tv_sec = 4, .tv_usec = 0};


struct ngadmin* ngadmin_init (const char *iface)
{
	struct ngadmin *nga;
	
	
	/* allocate main structure */
	nga = malloc(sizeof(struct ngadmin));
	memset(nga, 0, sizeof(struct ngadmin));
	nga->globalbroad = true;
	
	strncpy(nga->iface, iface, IFNAMSIZ - 1);
	
	if (startNetwork(nga) < 0) {
		free(nga);
		return NULL;
	}
	
	nga->timeout = default_timeout;
	if (updateTimeout(nga) < 0) {
		free(nga);
		return NULL;
	}
	
	
	return nga;
}


int ngadmin_close (struct ngadmin *nga)
{
	if (nga == NULL)
		return ERR_INVARG;
		
	stopNetwork(nga);
	free(nga->swi_tab);
	free(nga);
	
	return ERR_OK;
}


int ngadmin_forceInterface (struct ngadmin *nga)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	return forceInterface(nga) == 0 ? ERR_OK : ERR_NET;
}


int ngadmin_setKeepBroadcasting (struct ngadmin *nga, bool value)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	nga->keepbroad = value;
	
	return ERR_OK;
}


int ngadmin_useGlobalBroadcast (struct ngadmin *nga, bool value)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	if (setBroadcastType(nga, value) == 0)
		return ERR_OK;
	else
		return ERR_NET;
}


int ngadmin_setPassword (struct ngadmin *nga, const char *pass)
{
	if (nga == NULL)
		return ERR_INVARG;
	
	strncpy(nga->password, pass, PASSWORD_MAX);
	
	return ERR_OK;
}


int ngadmin_setTimeout (struct ngadmin *nga, const struct timeval *tv)
{
	int ret = ERR_OK;
	
	
	if (nga == NULL || tv == NULL)
		return ERR_INVARG;
	
	nga->timeout = *tv;
	if (updateTimeout(nga) < 0)
		ret = ERR_NET;
	
	
	return ret;
}


