
#include "commands.h"


int do_igmp_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i;
	struct igmp_conf ic;
	
	
	if (argc != 4) {
		printf("usage: igmp set <enable> <vlan> <validate> <block>\n");
		return 1;
	}
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	ic.enable = strtol(argv[0], NULL, 0);
	ic.vlan = strtol(argv[1], NULL, 0);
	ic.validate = strtol(argv[2], NULL, 0);
	ic.block = strtol(argv[3], NULL, 0);
	
	i = ngadmin_setIGMPConf(nga, &ic);
	printErrCode(i);
	
	
	return 0;
}


int do_igmp_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, ret = 0;
	const struct swi_attr *sa;
	struct igmp_conf ic;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		ret = 1;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	i = ngadmin_getIGMPConf(nga, &ic);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	printf("IGMP snooping enabled: %s\n", ic.enable ? "yes" : "no" );
	printf("IGMP snooping vlan: %u\n", ic.vlan);
	printf("Validate IGMPv3 headers: %s\n", ic.validate ? "yes" : "no" );
	printf("Block unknown multicast addresses: %s\n", ic.block ? "yes" : "no" );
	
end:
	
	return ret;
}


