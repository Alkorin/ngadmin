
#include "commands.h"


int do_firmware_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	int ret = 0;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		ret = 1;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	puts(sa->firmware);
	
end:
	
	return ret;
}


int do_firmware_upgrade (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	int i, ret = 0;
	
	
	if (argc != 1) {
		printf("usage: firmware upgrade <file>\n");
		ret = 1;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = 1;
		goto end;
	}
	
	i = ngadmin_upgradeFirmware(nga, argv[0]);
	printErrCode(i);
	
end:
	
	return ret;
}


