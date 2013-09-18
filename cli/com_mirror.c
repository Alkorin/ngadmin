
#include "commands.h"


bool do_mirror_disable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	int i;
	
	
	if (ngadmin_getCurrentSwitch(nga) == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	
	i = ngadmin_setMirror(nga, NULL);
	printErrCode(i);
	
	
	return true;
}


bool do_mirror_set (int nb, const char **com, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	char *ports = NULL;
	bool ret = true;
	int i, k = 0;
	
	
	if (nb < 3) {
		printf("Usage: mirror set <destination port> clone <port1> [<port2> ...]\n");
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	ports = malloc((sa->ports + 1) * sizeof(char));
	memset(ports, 0, sa->ports + 1);
	
	ports[0] = strtol(com[k++], NULL, 0);
	if (ports[0] < 1 || ports[0] > sa->ports || strcasecmp(com[k++], "clone") != 0) {
		printf("syntax error\n");
		ret = false;
		goto end;
	}
	
	while (k < nb) {
		i = strtol(com[k++], NULL, 0);
		if (i < 1 || i > sa->ports) {
			printf("port out of range\n");
			ret = false;
			goto end;
		} else if (i == ports[0]) {
			printf("destination port cannot be in port list\n");
			ret = false;
			goto end;
		}
		ports[i] = 1;
	}
	
	i = ngadmin_setMirror(nga, ports);
	printErrCode(i);
	
end:
	free(ports);
	
	return ret;
}


bool do_mirror_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	char *ports = NULL;
	int i;
	
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	
	ports = malloc((sa->ports + 1) * sizeof(char));
	i = ngadmin_getMirror(nga, ports);
	if (i != ERR_OK) {
		printErrCode(i);
		goto end;
	}
	
	if (ports[0] == 0) {
		printf("port mirroring is disabled\n");
		goto end;
	}
	
	printf("destination: %i\n", ports[0]);
	printf("ports: ");
	for (i = 1; i <= sa->ports; i++) {
		if (ports[i])
			printf("%i ", i);
	}
	printf("\n");
	
	
end:
	free(ports);
	
	return true;
}


