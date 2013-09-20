
#include "commands.h"


bool do_stormfilter_enable (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_setStormFilterState(nga, 1);
	printErrCode(i);
	
	
	return true;
}


bool do_stormfilter_disable (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_setStormFilterState(nga, 0);
	printErrCode(i);
	
	
	return true;
}


bool do_stormfilter_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i, d = BITRATE_UNSPEC, p, *ports = NULL;
	const struct swi_attr *sa;
	bool ret = true;
	
	
	if (argc < 2) {
		printf("usage: stormfilt set (all <speed0>)|(<port1> <speed1> [<port2> <speed2> ...])\n");
		ret = false;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	ports = malloc(sa->ports * sizeof(int));
	
	/* read defaults */
	if (strcmp(argv[0], "all") == 0) {
		d = parseBitrate(argv[1]);
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
		ports[p - 1] = parseBitrate(argv[i + 1]);
	}
	
	/* send the new configuration to the switch */
	i = ngadmin_setStormFilterValues(nga, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


bool do_stormfilter_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, s, ret = true, *ports = NULL;
	const struct swi_attr *sa;
	
	
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
	
	i = ngadmin_getStormFilterState(nga, &s);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
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
		ret = false;
		goto end;
	}
	
	for (i = 0; i < sa->ports; i++)
		printf("port %i: %s\n", i + 1, bitrates[ports[i]]);
	
end:
	free(ports);
	
	return ret;
}


