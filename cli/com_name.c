
#include "commands.h"


bool do_name_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	puts(sa->name);
	
	
	return true;
}


bool do_name_set (int argc, const char **argv, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc != 1) {
		printf("usage: name set <value>\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_setName(nga, argv[0]);
	printErrCode(i);
	
	
	return true;
}


bool do_name_clear (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return false;
	}
	
	i = ngadmin_setName(nga, NULL);
	printErrCode(i);
	
	
	return true;
}


