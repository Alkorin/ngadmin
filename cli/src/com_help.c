
#include "commands.h"


int do_help (int argc, const char **argv UNUSED, struct ngadmin *nga UNUSED)
{
	const struct TreeNode *s;
	
	
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	printf("Available commands: \n");
	
	for (s = commands.sub; s->name != NULL; s++)
		printf("%s ", s->name);
	putchar('\n');
	
	
	return 0;
}


