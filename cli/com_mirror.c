
#include "commands.h"


int do_mirror_disable (int argc, const char **argv UNUSED, struct ngadmin *nga)
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
	
	i = ngadmin_setMirror(nga, NULL);
	printErrCode(i);
	
	
	return 0;
}


int do_mirror_set (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	char *ports = NULL;
	int i, k = 0, ret = 0;
	
	
	if (argc < 3) {
		printf("usage: mirror set <destination port> clone <port1> [<port2> ...]\n");
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	ports = malloc((sa->ports + 1) * sizeof(char));
	memset(ports, 0, sa->ports + 1);
	
	ports[0] = strtol(argv[k++], NULL, 0);
	if (ports[0] < 1 || ports[0] > sa->ports || strcasecmp(argv[k++], "clone") != 0) {
		printf("syntax error\n");
		ret = 1;
		goto end;
	}
	
	while (k < argc) {
		i = strtol(argv[k++], NULL, 0);
		if (i < 1 || i > sa->ports) {
			printf("port out of range\n");
			ret = 1;
			goto end;
		} else if (i == ports[0]) {
			printf("destination port cannot be in port list\n");
			ret = 1;
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


int do_mirror_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	char *ports = NULL;
	int i;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
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
	
	return 0;
}


