
#include "commands.h"


int do_qos_mode (int argc, const char **argv, struct ngadmin *nga)
{
	int i, s, ret = 0;
	const struct swi_attr *sa;
	
	
	if (argc == 0) {
		printf("usage: qos mode port|802.1p\n");
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	if (strcasecmp(argv[0], "port") == 0) {
		s = QOS_PORT;
	} else if (strcasecmp(argv[0], "802.1p") == 0) {
		s = QOS_DOT;
	} else {
		printf("Unknown QOS mode\n");
		ret = 1;
		goto end;
	}
	
	i = ngadmin_setQOSMode(nga, s);
	printErrCode(i);
	
end:
	
	return ret;
}


int do_qos_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i, p, ret = 0;
	const struct swi_attr *sa;
	char d = PRIO_UNSPEC, *ports = NULL;
	
	
	if (argc < 2) {
		printf("usage: qos set (all <prio0>)|(<port1> <prio1> [<port2> <prio2> ...])\n");
		ret = 1;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa ==NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	ports = malloc(sa->ports * sizeof(char));
	
	/* read defaults */
	if (strcmp(argv[0], "all") == 0) {
		d = parseQosPrioStr(argv[1]);
		argv += 2;
		argc -= 2;
	}
	
	/* apply defaults */
	for (i = 0; i < sa->ports; i++)
		ports[i] = d;
	
	/* read and apply port specifics */
	for (i = 0; i < argc; i += 2) {
		p = strtol(argv[i], NULL, 0);
		if (p < 1 || p > sa->ports)
			continue;
		ports[p - 1] = parseQosPrioStr(argv[i + 1]);
	}
	
	/* send the new configuration to the switch */
	i = ngadmin_setQOSValues(nga, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


int do_qos_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, s = 0, ret = 0;
	const struct swi_attr *sa;
	char *ports = NULL;
	
	
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
	
	i = ngadmin_getQOSMode(nga, &s);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	printf("QoS mode: %s\n", safeStr(getQosTypeStr(s)));
	if (s != QOS_PORT)
		goto end;
	
	ports = malloc(sa->ports * sizeof(char));
	i = ngadmin_getQOSValues(nga, ports);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	for (i = 0; i < sa->ports; i++)
		printf("port %i: %s\n", i + 1, safeStr(getQosPrioStr(ports[i])));
	
end:
	free(ports);
	
	return ret;
}


