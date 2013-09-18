
#include "commands.h"


bool do_igmp_set (int nb, const char **com, struct ngadmin *nga)
{
	int i;
	struct igmp_conf ic;
	
	
	if (nb != 4) {
		printf("Usage: igmp set <enable> <vlan> <validate> <block>\n");
		return false;
	}
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	ic.enable = strtol(com[0], NULL, 0);
	ic.vlan = strtol(com[1], NULL, 0);
	ic.validate = strtol(com[2], NULL, 0);
	ic.block = strtol(com[3], NULL, 0);
	
	i = ngadmin_setIGMPConf(nga, &ic);
	printErrCode(i);
	
	
	return true;
}


bool do_igmp_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	struct igmp_conf ic;
	bool ret = true;
	
	
	sa=ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	i = ngadmin_getIGMPConf(nga, &ic);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
		goto end;
	}
	
	printf("IGMP snooping enabled: %s\n", ic.enable ? "yes" : "no" );
	printf("IGMP snooping vlan: %u\n", ic.vlan);
	printf("Validate IGMPv3 headers: %s\n", ic.validate ? "yes" : "no" );
	printf("Block unknown multicast addresses: %s\n", ic.block ? "yes" : "no" );
	
end:
	
	return ret;
}


