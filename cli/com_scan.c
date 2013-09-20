
#include "commands.h"


bool do_scan (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	i = ngadmin_scan(nga);
	if (i < 0) {
		printErrCode(i);
		return false;
	}
	
	sa = ngadmin_getSwitchTab(nga, &i);
	displaySwitchTab(sa, i);
	
	
	return true;
}


