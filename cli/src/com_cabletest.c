
#include "commands.h"


int do_cabletest (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	struct cabletest *ct = NULL;
	int i, j = 0, k = 0, ret = 0;
	
	
	if (argc < 1) {
		printf("usage: cabletest <port1> [<port2> ...]\n");
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	ct = malloc(sa->ports * sizeof(struct cabletest));
	memset(ct, 0, sa->ports * sizeof(struct cabletest));
	
	while (k < argc) {
		ct[j].port = strtol(argv[k++], NULL, 0);
		if (ct[j].port >= 1 && ct[j].port <= sa->ports)
			j++;
	}
	
	i = ngadmin_cabletest(nga, ct, j);
	if (i < 0) {
		printErrCode(i);
		ret = 1;
		goto end;
	}
	
	for (i = 0; i < j; i++)
		printf("port %i: %s, code=%d, distance=%d m\n", ct[i].port, getCableTestResultStr(ct[i].test_result), ct[i].test_result, ct[i].fault_distance);
	
end:
	free(ct);
	
	return ret;
}


