
#include "commands.h"


bool do_list (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	int n;
	const struct swi_attr *sa;
	
	sa = ngadmin_getSwitchTab(nga, &n);
	displaySwitchTab(sa, n);
	
	return true;
}


