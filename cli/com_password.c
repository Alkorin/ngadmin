
#include "commands.h"



bool do_password_change (int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 
 
 if ( nb!=1 ) {
  printf("Usage: password change <value>\n");
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



bool do_password_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 
 
 if ( nb!=1 ) {
  printf("Usage: password set <value>\n");
  return false;
 }
 
 i=ngadmin_setPassword(nga, com[0]);
 printErrCode(i);
 
 
 return true;
 
}



