
#include "commands.h"




bool do_help (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga UNUSED) {
 
 const struct TreeNode *s;
 
 
 printf("Available commands: \n");
 
 for (s=coms.sub; s->name!=NULL; ++s) {
  printf("%s ", s->name);
 }
 putchar('\n');
 
 
 return true;
 
}




