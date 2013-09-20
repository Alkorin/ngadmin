
#include "commands.h"


bool do_defaults (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, ret = true;
	const struct swi_attr *sa;
	char line[16];
	
	
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
	
	printf("The switch settings will be CLEARED. Continue ? [y/N]: ");
	fflush(stdout);
	
	if (fgets(line, sizeof(line), stdin) != NULL && strcasecmp(line, "y\n") == 0) {
		i = ngadmin_defaults(nga);
		printErrCode(i);
	}
	
end:
	
	return ret;
}


