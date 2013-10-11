
#include "commands.h"


int do_scan (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	i = ngadmin_scan(nga);
	if (i < 0) {
		printErrCode(i);
		return 1;
	}
	
	sa = ngadmin_getSwitchTab(nga, &i);
	displaySwitchTab(sa, i);
	
	
	return 0;
}


