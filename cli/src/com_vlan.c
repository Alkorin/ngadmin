
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


int do_vlan_8021q_del (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	unsigned short vlan;
	int i;
	
	
	if (argc != 1) {
		printf("usage: vlan 8021q del <vlan>\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	vlan=strtoul(argv[0], NULL, 0);
	if (vlan < VLAN_MIN || vlan > VLAN_DOT_MAX) {
		printf("vlan out of range\n");
		return 1;
	}
	
	i = ngadmin_VLANDestroy(nga, vlan);
	printErrCode(i);
	
	
	return 0;
}


int do_vlan_port_set (int argc, const char **argv, struct ngadmin *nga)
{
	unsigned char vlan, port, *ports = NULL;
	const struct swi_attr *sa;
	int i, k = 0, ret = 0;
	
	
	if (argc < 2) {
		printf("usage: vlan port set [all <vlan>] [<port1> <vlan>] [<port2> <vlan>] [...]\n");
		ret = 1;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	ports = malloc(sa->ports * sizeof(unsigned char));

	/* read defaults */
	vlan = 0;
	if (strcmp(argv[k], "all") == 0) {
		k++;
		vlan = strtoul(argv[k++], NULL, 0);
		/* VLAN 0 is allowed and means no change */
		if (vlan > VLAN_PORT_MAX) {
			printf("vlan out of range\n");
			ret = 1;
			goto end;
		}
	}
	
	/* apply defaults */
	memset(ports, vlan, sa->ports);
	
	/* read and apply port specifics */
	while (k < argc - 1) {
		/* read port */
		port = strtoul(argv[k++], NULL, 0);
		if (port < 1 || port > sa->ports) {
			printf("port out of range\n");
			ret = 1;
			goto end;
		}
		
		/* read vlan */
		vlan = strtoul(argv[k++], NULL, 0);
		/* VLAN 0 is allowed and means no change */
		if (vlan > VLAN_PORT_MAX) {
			printf("vlan out of range\n");
			ret = 1;
			goto end;
		}
		
		ports[port - 1] = vlan;
	}
	
	/* set conf */
	i = ngadmin_setVLANPortConf(nga, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


int do_vlan_port_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	unsigned char *ports = NULL;
	const struct swi_attr *sa;
	int i, ret = 0;
	
	
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
	
	ports = malloc(sa->ports * sizeof(unsigned char));
	
	/* request all VLANs config */
	i = ngadmin_getVLANPortConf(nga, ports);
	
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	printf("Ports configuration: \n");
	printf("Port\t");
	for (i = 1; i <= sa->ports; i++)
		printf("%i\t", i);
	putchar('\n');
	
	/* show all VLANs */
	printf("VLAN\t");
	for (i = 0; i < sa->ports; i++)
		printf("%u\t", ports[i]);
	putchar('\n');
	
end:
	free(ports);
	
	return ret;
}


int do_vlan_8021q_set (int argc, const char **argv, struct ngadmin *nga)
{
	unsigned char *ports = NULL, p, def = VLAN_UNSPEC;
	const struct swi_attr *sa;
	unsigned short vlan;
	int i, k = 0, ret = 0;
	
	
	if (argc == 0) {
		printf("usage: vlan 802.1q set <vlan> [all unspec|no|untagged|tagged] [<port1> unspec|no|untagged|tagged ...]\n");
		ret = 1;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	/* read vlan */
	vlan = strtoul(argv[k++], NULL, 0);
	
	if (vlan < VLAN_MIN || vlan > VLAN_DOT_MAX) {
		printf("vlan out of range\n");
		ret = 1;
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
			ret = 1;
			goto end;
		}
		k++;
	}
	
	ports = malloc(sa->ports * sizeof(unsigned char));
	
	/* apply defaults */
	memset(ports, def, sa->ports);
	
	/* read and apply port specifics */
	while (k < argc - 1) {
		p = strtoul(argv[k++], NULL, 0) - 1;
		if (p >= sa->ports) {
			printf("port out of range\n");
			ret = 1;
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
			ret = 1;
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


int do_vlan_8021q_show (int argc, const char **argv, struct ngadmin *nga)
{
	unsigned short vl = 0, *vlans = NULL;
	unsigned char *ports = NULL;
	const struct swi_attr *sa;
	int i, j, n = 16, ret = 0;
	
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
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
		ret = 1;
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


int do_vlan_mode_set (int argc, const char **argv, struct ngadmin *nga)
{
	int mode, i;
	
	
	if (argc == 0) {
		printf(
		"usage: vlan mode set <mode>\n"
		"0 - disabled\n"
		"1 - basic port based\n"
		"2 - advanced port based\n"
		"3 - basic 802.1Q\n"
		"4 - advanced 802.1Q\n"
		);
		return 0;
	}
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	mode = strtoul(argv[0], NULL, 0);
	if (mode < VLAN_DISABLED || mode > VLAN_DOT_ADV) {
		printf("mode out of range\n");
		return 1;
	}
	
	i = ngadmin_setVLANType(nga, mode);
	printErrCode(i);
	
	
	return 0;
}


int do_vlan_mode_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, t, ret = 0;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		ret = 1;
		goto end;
	}
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	i = ngadmin_getVLANType(nga, &t);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
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


int do_vlan_pvid_set (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	unsigned char port;
	unsigned short vlan;
	int i;
	
	
	if (argc != 2) {
		printf("usage: vlan pvid set <port> <vlan>\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	port = strtoul(argv[0], NULL, 0);
	vlan = strtoul(argv[1], NULL, 0);
	
	if (port < 1 || port > sa->ports) {
		printf("port out of range\n");
		return 1;
	}
	
	if (vlan < VLAN_MIN || vlan > VLAN_DOT_MAX) {
		printf("vlan out of range\n");
		return 1;
	}
	
	i = ngadmin_setPVID(nga, port, vlan);
	printErrCode(i);
	
	
	return 0;
}


int do_vlan_pvid_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	unsigned short *ports = NULL;
	const struct swi_attr *sa;
	int i, ret = 0;
	
	
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
	
	ports = malloc(sa->ports * sizeof(unsigned short));
	i = ngadmin_getAllPVID(nga, ports);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
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


