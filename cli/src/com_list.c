
#include "commands.h"


int do_list (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int n;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	sa = ngadmin_getSwitchTab(nga, &n);
	displaySwitchTab(sa, n);
	
	
	return 0;
}


