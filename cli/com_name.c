
#include "commands.h"


bool do_name_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	puts(sa->name);
	
	
	return true;
}


bool do_name_set (int nb, const char **com, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (nb != 1) {
		printf("Usage: name set <value>\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_setName(nga, com[0]);
	printErrCode(i);
	
	
	return true;
}


bool do_name_clear (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_setName(nga, NULL);
	printErrCode(i);
	
	
	return true;
}


