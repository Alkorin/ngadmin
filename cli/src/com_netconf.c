
#include "commands.h"


int do_netconf_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	const struct net_conf *nc;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	nc = &sa->nc;
	
	printf("DHCP\t : %s\n", nc->dhcp ? "yes" : "no");
	printf("IP\t : %s\n", inet_ntoa(nc->ip));
	printf("Netmask\t : %s\n", inet_ntoa(nc->netmask));
	printf("Gateway\t : %s\n", inet_ntoa(nc->gw));
	
	
	return 0;
}


int do_netconf_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i, k, ret = 0;
	const struct swi_attr *sa;
	struct net_conf nc;
	
	
	if (argc == 0) {
		printf("usage: netconf set [dhcp yes|no] [ip <ip>] [mask <mask>] [gw <gw>]\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	memset(&nc, 0, sizeof(struct net_conf));
	
	for (k = 0; k < argc; k += 2) {
		if (strcasecmp(argv[k], "dhcp") == 0) {
			if (strcasecmp(argv[k + 1], "yes") == 0) {
				nc.dhcp = true;
			} else if (strcasecmp(argv[k + 1], "no") == 0) {
				nc.dhcp = 1;
			} else {
				printf("Incorrect DHCP value\n");
				ret = 1;
				goto end;
			}
		} else if (strcasecmp(argv[k], "ip") == 0) {
			if (inet_aton(argv[k + 1], &nc.ip) == 0) {
				printf("Incorrect IP value\n");
				ret = 1;
				goto end;
			}
		} else if (strcasecmp(argv[k], "mask") == 0) {
			/* TODO: check if it is a correct mask */
			if (inet_aton(argv[k + 1], &nc.netmask) == 0) {
				printf("Incorrect mask value\n");
				ret = 1;
				goto end;
			}
		} else if (strcasecmp(argv[k], "gw") == 0) {
			if (inet_aton(argv[k + 1], &nc.gw) == 0) {
				printf("Incorrect gateway value\n");
				ret = 1;
				goto end;
			}
		}
	}
	
	i = ngadmin_setNetConf(nga, &nc);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
	}
	
end:
	
	return ret;
}


