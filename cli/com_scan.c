
#include "commands.h"


bool do_scan (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	i = ngadmin_scan(nga);
	if (i < 0) {
		printErrCode(i);
		return false;
	}
	
	sa = ngadmin_getSwitchTab(nga, &nb);
	displaySwitchTab(sa, nb);
	
	
	return true;
}


