
#include "commands.h"



static char vlan_char (int t)
{
	switch (t) {
	
	case VLAN_TAGGED:
		return 'T';
	
	case VLAN_UNTAGGED:
		return 'U';
	
	case VLAN_NO:
		return ' ';
	
	default:
		return '?';
	}
}


bool do_vlan_8021q_del (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	unsigned short vlan;
	int i;
	
	
	if (argc != 1) {
		printf("usage: vlan 8021q del <vlan>\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	vlan=strtoul(argv[0], NULL, 0);
	if (vlan < 1 || vlan > VLAN_MAX) {
		printf("vlan out of range\n");
		return false;
	}
	
	i = ngadmin_VLANDestroy(nga, vlan);
	printErrCode(i);
	
	
	return true;
}


bool do_vlan_8021q_set (int argc, const char **argv, struct ngadmin *nga)
{
	unsigned char *ports = NULL, p, def = VLAN_UNSPEC;
	const struct swi_attr *sa;
	bool ret = true;
	unsigned short vlan;
	int i, k = 0;
	
	
	if (argc == 0) {
		printf("usage: vlan 802.1q set <vlan> [all unspec|no|untagged|tagged] [<port1> unspec|no|untagged|tagged ...]\n");
		ret = false;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	/* read vlan */
	vlan = strtoul(argv[k++], NULL, 0);
	
	if (vlan < 1 || vlan > VLAN_MAX) {
		printf("vlan out of range\n");
		ret = false;
		goto end;
	}
	
	/* read defaults */
	if (k < argc - 1 && strcasecmp(argv[k], "all") == 0) {
		k++;
		if (strcasecmp(argv[k], "tagged") == 0) {
			def = VLAN_TAGGED;
		} else if (strcasecmp(argv[k], "untagged") == 0) {
			def = VLAN_UNTAGGED;
		} else if (strcasecmp(argv[k], "no") == 0) {
			def = VLAN_NO;
		} else if (strcasecmp(argv[k], "unspec") == 0) {
			def = VLAN_UNSPEC;
		} else {
			printf("incorrect type\n");
			ret = false;
			goto end;
		}
		k++;
	}
	
	ports = malloc(sa->ports * sizeof(unsigned char));
	
	/* apply defaults */
	memset(ports, def, sa->ports);
	
	/* apply port specifics */
	while (k < argc - 1) {
		p = strtoul(argv[k++], NULL, 0) - 1;
		if (p >= sa->ports) {
			printf("port out of range\n");
			ret = false;
			goto end;
		}
		if (strcasecmp(argv[k], "tagged") ==0) {
			ports[p] = VLAN_TAGGED;
		} else if (strcasecmp(argv[k], "untagged") == 0) {
			ports[p] = VLAN_UNTAGGED;
		} else if (strcasecmp(argv[k], "no") == 0) {
			ports[p] = VLAN_NO;
		} else if (strcasecmp(argv[k], "unspec") == 0) {
			ports[p] = VLAN_UNSPEC;
		} else {
			printf("incorrect type\n");
			ret = false;
			goto end;
		}
		k++;
	}
	
	/* set conf */
	i = ngadmin_setVLANDotConf(nga, vlan, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


bool do_vlan_8021q_show (int argc, const char **argv, struct ngadmin *nga)
{
	unsigned short vl = 0, *vlans = NULL;
	unsigned char *ports = NULL;
	const struct swi_attr *sa;
	int i, j, n = 16;
	bool ret = true;
	
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	if (argc > 0)
		vl = strtoul(argv[0], NULL, 0);
	
	ports = malloc(sa->ports * n * sizeof(unsigned char));
	
	if (vl == 0) {
		/* request all VLANs config */
		vlans = malloc(n * sizeof(unsigned short));
		ports = malloc(sa->ports * n * sizeof(unsigned char));
		i = ngadmin_getVLANDotAllConf(nga, vlans, ports, &n);
	} else {
		/* request single VLAN config */
		ports = malloc(sa->ports * sizeof(unsigned char));
		i = ngadmin_getVLANDotConf(nga, vl, ports);
	}
	
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
		goto end;
	}
	
	printf("Ports configuration: \n");
	printf("VLAN\t");
	for (i = 1; i <= sa->ports; i++)
		printf("%i\t", i);
	putchar('\n');
	
	if (vl == 0) {
		/* show all VLANs */
		for (i = 0; i < n; i++) {
			printf("%u\t", vlans[i]);
			for (j = 0; j < sa->ports; j++)
				printf("%c\t", vlan_char(ports[i * sa->ports + j]));
			putchar('\n');
		}
	} else {
		/* show single VLAN config */
		printf("%u\t", vl);
		for (j = 0; j < sa->ports; j++)
			printf("%c\t", vlan_char(ports[j]));
		putchar('\n');
	}
	
end:
	free(vlans);
	free(ports);
	
	return ret;
}


bool do_vlan_mode_set (int argc, const char **argv, struct ngadmin *nga)
{
	int mode, i;
	
	
	if (argc == 0) {
		printf(
		"usage: vlan mode set <mode>\n"
		"1 - basic port based\n"
		"2 - advanced port based\n"
		"3 - basic 802.1Q\n"
		"4 - advanced 802.1Q\n"
		);
		return true;
	}
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	mode = strtoul(argv[0], NULL, 0);
	if (mode < 1 || mode > 4) {
		printf("mode out of range\n");
		return false;
	}
	
	i = ngadmin_setVLANType(nga, mode);
	printErrCode(i);
	
	
	return true;
}


bool do_vlan_mode_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, t, ret = true;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		ret = false;
		goto end;
	}
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	i = ngadmin_getVLANType(nga, &t);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
		goto end;
	}
	
	printf("VLAN type: ");
	switch (t) {
	
	case VLAN_DISABLED:
		printf("disabled\n");
		break;
	
	case VLAN_PORT_BASIC:
		printf("port basic\n");
		break;
	
	case VLAN_PORT_ADV:
		printf("port advanced\n");
		break;
	
	case VLAN_DOT_BASIC:
		printf("802.1Q basic\n");
		break;
	
	case VLAN_DOT_ADV:
		printf("802.1Q advanced\n");
		break;
	
	default:
		printf("unknown (%i)\n", t);
	}
	
end:
	
	return ret;
}


bool do_vlan_pvid_set (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	unsigned char port;
	unsigned short vlan;
	int i;
	
	
	if (argc != 2) {
		printf("usage: vlan pvid set <port> <vlan>\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	port = strtoul(argv[0], NULL, 0);
	vlan = strtoul(argv[1], NULL, 0);
	
	if (port < 1 || port > sa->ports) {
		printf("port out of range\n");
		return false;
	}
	
	if (vlan < 1 || vlan > VLAN_MAX) {
		printf("vlan out of range\n");
		return false;
	}
	
	i = ngadmin_setPVID(nga, port, vlan);
	printErrCode(i);
	
	
	return true;
}


bool do_vlan_pvid_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	unsigned short *ports = NULL;
	const struct swi_attr *sa;
	int i;
	bool ret = true;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		ret = false;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	ports = malloc(sa->ports * sizeof(unsigned short));
	i = ngadmin_getAllPVID(nga, ports);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
		goto end;
	}
	
	printf("Port\t");
	for (i = 1; i <= sa->ports; i++)
		printf("%i\t", i);
	putchar('\n');
	
	printf("VLAN\t");
	for (i = 0; i < sa->ports; i++)
		printf("%u\t", ports[i]);
	putchar('\n');
	
end:
	free(ports);
	
	return ret;
}


