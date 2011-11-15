
#include "common.h"




static bool do_password_change (const struct TreeNode *tn UNUSED, int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 
 
 if ( nb!=1 ) {
  printf("Usage: password set <value>\n");
  return false;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_changePassword(nga, com[0]);
 printErrCode(i);
 
 
 return true;
 
}



static bool do_password_set (const struct TreeNode *tn UNUSED, int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 
 
 if ( nb!=1 ) {
  printf("Usage: password set <value>\n");
  return false;
 }
 
 i=ngadmin_setPassword(nga, com[0]);
 printErrCode(i);
 
 
 return true;
 
}




static const struct TreeNode com_password_change=COM("change", do_password_change, true, NULL);
static const struct TreeNode com_password_set=COM("set", do_password_set, true, NULL);

const struct TreeNode com_password=COM("password", NULL, false, &com_password_change, &com_password_set, NULL);



