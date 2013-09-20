
#include "commands.h"


bool do_cabletest (int argc, const char **argv, struct ngadmin *nga)
{
	bool ret = true;
	const struct swi_attr *sa;
	struct cabletest *ct = NULL;
	int i, j=0, k=0;
	
	
	if (argc < 1) {
		printf("usage: cabletest <port1> [<port2> ...]\n");
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
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
		ret = false;
		goto end;
	}
	
	for (i = 0; i < j; i++)
		printf("port %i: %08X %08X\n", ct[i].port, ct[i].v1, ct[i].v2);
	
end:
	free(ct);
	
	return ret;
}


