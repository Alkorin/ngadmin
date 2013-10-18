
#include "commands.h"


int do_login (int argc, const char **argv, struct ngadmin *nga)
{
	int i;
	
	
	if (argc != 1) {
		printf("usage: login <num>\n");
		return 1;
	}
	
	i = strtol(argv[0], NULL, 0);
	i = ngadmin_login(nga, i);
	printErrCode(i);
	
	
	return 0;
}


