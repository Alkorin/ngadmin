
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
	
	for (i = 0; i < sa->ports; i++) {
		printf("port %i: ", i + 1);
		switch (ports[i]) {
		
		case 0:
			printf("down");
			break;
		
		case SPEED_10:
			printf("up, 10M");
			break;
		
		case SPEED_100:
			printf("up, 100M");
			break;
		
		case SPEED_1000:
			printf("up, 1000M");
			break;
		
		default:
			printf("unknown (%i)", ports[i]);
		}
		putchar('\n');
	}
	
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
	
	printf("Port\tReceived\tSent\tCRC errors\n");
	for (i = 0; i < sa->ports; i++)
		printf("% 4i%12llu%12llu%14llu\n", i + 1, ps[i].recv, ps[i].sent, ps[i].crc);
	
end:
	free(ps);
	
	return ret;
}


