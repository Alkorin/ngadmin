
#include "commands.h"


int do_loop_enable (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	i = ngadmin_setLoopDetectionState(nga, 1);
	printErrCode(i);
	
	
	return 0;
}


int do_loop_disable (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	i = ngadmin_setLoopDetectionState(nga, 0);
	printErrCode(i);
	
	
	return 0;
}


int do_loop_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	int i, s;
	const struct swi_attr *sa;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		return 1;
	}
	
	i = ngadmin_getLoopDetectionState(nga, &s);
	if (i != ERR_OK) {
		printErrCode(i);
		return 1;
	}
	
	printf("loop detection is %s\n", s ? "enabled" : "disabled");
	
	return 0;
}


