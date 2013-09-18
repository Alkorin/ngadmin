
#include "commands.h"


bool do_restart (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga UNUSED)
{
	int i, ret = true;
	const struct swi_attr *sa;
	char line[16];
	
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	printf("The switch will be restarted. Continue ? [y/N]: ");
	fflush(stdout);
	
	if (fgets(line, sizeof(line), stdin) != NULL && strcasecmp(line, "y\n") == 0) {
		i = ngadmin_restart(nga);
		printErrCode(i);
	}
	
end:
	
	return ret;
}


