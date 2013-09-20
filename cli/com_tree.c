
#include "commands.h"


static void display_node (const struct TreeNode *tn, int depth)
{
	int i;
	const struct TreeNode *s;
	
	
	for (i = 0; i < depth; i++)
		putchar('\t');
	puts(tn->name);
	
	if (tn->sub == NULL)
		return;
	
	for (s = tn->sub; s->name != NULL; s++)
		display_node(s, depth + 1);
}


int do_tree (int argc, const char **argv UNUSED, struct ngadmin *nga UNUSED)
{
	if (argc > 0) {
		printf("this command takes no argument\n");
		return 1;
	}
	
	display_node(&commands, 0);
	
	return 0;
}


