
#include "common.h"




static bool do_name_show (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 const struct swi_attr *sa;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 puts(sa->name);
 
 
 return true;
 
}



static bool do_name_set (const struct TreeNode *tn UNUSED, int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 
 
 if ( nb!=1 ) {
  printf("Usage: name set <value>\n");
  return false;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_setName(nga, com[0]);
 printErrCode(i);
 
 
 return true;
 
}



static bool do_name_clear (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_setName(nga, NULL);
 printErrCode(i);
 
 
 return true;
 
}



static struct TreeNode com_name_show=COM("show", do_name_show, false, NULL);
static struct TreeNode com_name_set=COM("set", do_name_set, true, NULL);
static struct TreeNode com_name_clear=COM("clear", do_name_clear, false, NULL);

const struct TreeNode com_name=COM("name", NULL, false, &com_name_show, &com_name_set, &com_name_clear, NULL);



