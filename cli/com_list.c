
#include "commands.h"


bool do_list (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int n;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	sa = ngadmin_getSwitchTab(nga, &n);
	displaySwitchTab(sa, n);
	
	return true;
}


