
#include "commands.h"


/* helper function to analyse bitrate speed specifications */
static int bitrate_analyse (int argc, const char **argv, int *ports)
{
	int i = 0, s;
	
	
	while (i < argc - 1) {
		s = parseBitrate(argv[i + 1]);
		if (strcmp(argv[i], "inout") == 0) {
			ports[0] = s;
			ports[1] = s;
		} else if (strcmp(argv[i], "in") == 0) {
			ports[0] = s;
		} else if (strcmp(argv[i], "out") == 0) {
			ports[1] = s;
		} else {
			break;
		}
		i += 2;
	}
	
	
	return i;
}


bool do_bitrate_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i, k = 0, defs[] = {12, 12}, p, *ports = NULL;
	const struct swi_attr *sa;
	bool ret = true;
	
	if (argc < 2) {
		printf(
		"usage: bitrate set [all SPEEDSPEC] <port1> SPEEDSPEC [<port2> SPEEDSPEC ...]\n"
		"SPEEDSPEC: [inout <speed>] [in <ispeed>] [out <ospeed>]\n"
		);
		ret = false;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	ports = malloc(2 * sa->ports * sizeof(int));
	
	/* get defaults if present */
	if (strcmp(argv[k], "all") == 0) {
		k++;
		k += bitrate_analyse(argc - k, &argv[k], defs);
	}
	
	/* apply defaults */
	for (i = 0; i < sa->ports; i++)
		memcpy(&ports[2 * i], defs, sizeof(defs));
	
	/* get ports specifics */
	while (k < argc) {
		p = strtol(argv[k++], NULL, 0) - 1;
		if (p >= 0 &&  p <sa->ports)
			k += bitrate_analyse(argc - k, &argv[k], &ports[2 * p]);
	}
	
	/* send it to the switch */
	i = ngadmin_setBitrateLimits(nga, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


bool do_bitrate_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, ret = true, *ports = NULL;
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
	
	
	ports = malloc(2 * sa->ports * sizeof(int));
	i = ngadmin_getBitrateLimits(nga, ports);
	if (i != ERR_OK) {
		printErrCode(i);
		ret = false;
		goto end;
	}
	
	for (i = 0; i < sa->ports; i++)
		printf("port %i: in %s, out %s\n", i + 1, bitrates[ports[2 * i + 0]], bitrates[ports[2 * i + 1]]);
	
end:
	free(ports);
	
	return ret;
}



