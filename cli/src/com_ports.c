
#include "commands.h"


int do_ports_state (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, ret = 0;
	const struct swi_attr *sa;
	unsigned char *ports = NULL;
	
	
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
	i = ngadmin_getPortsStatus(nga, ports);
	if (i < 0) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	for (i = 0; i < sa->ports; i++)
		printf("port %i: %s\n", i + 1, safeStr(getSpeedStr(ports[i])));
	
end:
	free(ports);
	
	
	return ret;
}


int do_ports_statistics_reset (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	i = ngadmin_resetPortsStatistics(nga);
	printErrCode(i);
	
	return 0;
}


int do_ports_statistics_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, ret = 0;
	const struct swi_attr *sa;
	struct port_stats *ps = NULL;
	
	
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
	
	ps = calloc(sa->ports, sizeof(struct port_stats));
	i = ngadmin_getPortsStatistics(nga, ps);
	if (i < 0) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	printf("Port             Received                 Sent           CRC errors\n");
	for (i = 0; i < sa->ports; i++)
		printf("% 4i%21llu%21llu%21llu\n", i + 1, ps[i].recv, ps[i].sent, ps[i].crc);
	
end:
	free(ps);
	
	return ret;
}


