
#include "commands.h"


bool do_qos_mode (int nb, const char **com, struct ngadmin *nga)
{
	int i, s, ret = true;
	const struct swi_attr *sa;
	
	
	if (nb == 0) {
		printf("Usage: qos mode port|802.1p\n");
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	if (strcasecmp(com[0], "port") == 0) {
		s = QOS_PORT;
	} else if (strcasecmp(com[0], "802.1p") == 0) {
		s = QOS_DOT;
	} else {
		printf("Unknown QOS mode\n");
		ret = false;
		goto end;
	}
	
	i = ngadmin_setQOSMode(nga, s);
	printErrCode(i);
	
end:
	
	return ret;
}


bool do_qos_set (int nb, const char **com, struct ngadmin *nga)
{
	int i, p;
	const struct swi_attr *sa;
	bool ret = true;
	char d = PRIO_UNSPEC, *ports = NULL;
	
	
	if (nb < 2) {
		printf("Usage: qos set (all <prio0>)|(<port1> <prio1> [<port2> <prio2> ...])\n");
		ret = false;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa ==NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	ports = malloc(sa->ports * sizeof(char));
	
	if (strcmp(com[0], "all") == 0) {
		d = parsePrio(com[1]);
		com += 2;
		nb -= 2;
	}
	
	for (i = 0; i < sa->ports; i++)
		ports[i] = d;
	
	for (i = 0; i < nb; i += 2) {
		p = strtol(com[i], NULL, 0);
		if (p < 1 || p > sa->ports)
			continue;
		ports[p - 1] = parsePrio(com[i + 1]);
	}
	
	i = ngadmin_setQOSValues(nga, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


bool do_qos_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	int i, s = 0, ret = true;
	const struct swi_attr *sa;
	char *ports = NULL;
	
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	i = ngadmin_getQOSMode(nga, &s);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
		goto end;
	}
	
	printf("QoS mode: ");
	switch (s) {
	
	case QOS_DOT:
		printf("802.1p\n");
		goto end;
	
	case QOS_PORT:
		printf("port based\n");
		break;
		
	default:
		printf("unknown (%i)\n", s);
		goto end;
	}
	
	ports = malloc(sa->ports * sizeof(char));
	i = ngadmin_getQOSValues(nga, ports);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
		goto end;
	}
	
	for (i = 0; i < sa->ports; i++)
		printf("port %i: %s\n", i + 1, prio[(int)ports[i]]);
	
end:
	free(ports);
	
	return ret;
}


