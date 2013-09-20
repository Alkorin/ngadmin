
#include "commands.h"


int do_quit (int argc, const char **argv UNUSED, struct ngadmin *nga UNUSED)
{
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	main_loop_continue = 0;
	
	return 0;
}


