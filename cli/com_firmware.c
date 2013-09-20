
#include "commands.h"


bool do_firmware_show (int argc, const char **argv UNUSED, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	bool ret = true;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		ret = false;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	puts(sa->firmware);
	
end:
	
	return ret;
}


bool do_firmware_upgrade (int argc, const char **argv, struct ngadmin *nga)
{
	const struct swi_attr *sa;
	bool ret = true;
	int i;
	
	
	if (argc != 1) {
		printf("usage: firmware upgrade <file>\n");
		ret = false;
		goto end;
	}
	
	sa = ngadmin_getCurrentSwitch(nga);
	if (sa == NULL) {
		printf("must be logged\n");
		ret = false;
		goto end;
	}
	
	i = ngadmin_upgradeFirmware(nga, argv[0]);
	printErrCode(i);
	
end:
	
	return ret;
}


