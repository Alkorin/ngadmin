
#include "commands.h"


bool do_quit (int argc, const char **argv UNUSED, struct ngadmin *nga UNUSED)
{
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	main_loop_continue = 0;
	
	return true;
}


