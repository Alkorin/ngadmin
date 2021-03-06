
#include "commands.h"


int do_stormfilter_enable (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	i = ngadmin_setStormFilterState(nga, 1);
	printErrCode(i);
	
	
	return 0;
}


int do_stormfilter_disable (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	i = ngadmin_setStormFilterState(nga, 0);
	printErrCode(i);
	
	
	return 0;
}


int do_stormfilter_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i, d = BITRATE_UNSPEC, p, *ports = NULL, ret = 0;
	const struct swi_attr *sa;
	
	
	if (argc < 2) {
		printf("usage: stormfilt set (all <speed0>)|(<port1> <speed1> [<port2> <speed2> ...])\n");
		ret = 1;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	ports = malloc(sa->ports * sizeof(int));
	
	/* read defaults */
	if (strcmp(argv[0], "all") == 0) {
		d = parseBitrateStr(argv[1]);
		argv += 2;
		argc -= 2;
	}
	
	/* apply defaults */
	for (i = 0; i < sa->ports; i++)
		ports[i] = d;
	
	/* read and apply port specifics */
	for (i = 0; i < argc - 1; i += 2) {
		p = strtol(argv[i], NULL, 0);
		if (p < 1 || p > sa->ports)
			continue;
		ports[p - 1] = parseBitrateStr(argv[i + 1]);
	}
	
	/* send the new configuration to the switch */
	i = ngadmin_setStormFilterValues(nga, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


int do_stormfilter_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, s, ret = 0, *ports = NULL;
	const struct swi_attr *sa;
	
	
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
	
	i = ngadmin_getStormFilterState(nga, &s);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	if (!s) {
		printf("storm filter is disabled\n");
		goto end;
	}
	
	printf("storm filter is enabled\n");
	
	ports = malloc(sa->ports * sizeof(int));
	i = ngadmin_getStormFilterValues(nga, ports);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	for (i = 0; i < sa->ports; i++)
		printf("port %i: %s\n", i + 1, safeStr(getBitrateStr(ports[i])));
	
end:
	free(ports);
	
	return ret;
}


