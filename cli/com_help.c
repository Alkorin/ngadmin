
#include "commands.h"


bool do_help (int argc, const char **argv UNUSED, struct ngadmin *nga UNUSED)
{
	const struct TreeNode *s;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return false;
	}
	
	printf("Available commands: \n");
	
	for (s = commands.sub; s->name != NULL; s++)
		printf("%s ", s->name);
	putchar('\n');
	
	return true;
}


