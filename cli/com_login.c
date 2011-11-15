
#include "common.h"




static bool do_login (const struct TreeNode *tn UNUSED, int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 
 
 if ( nb!=1 ) {
  printf("Usage: login <num>\n");
  return false;
 }
 
 
 i=strtol(com[0], NULL, 0);
 i=ngadmin_login(nga, i);
 printErrCode(i);
 
 
 return true;
 
}



const struct TreeNode com_login=COM("login", do_login, true, NULL);



