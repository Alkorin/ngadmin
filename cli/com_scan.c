
#include "common.h"




static bool do_scan (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 
 
 if ( (i=ngadmin_scan(nga))<0 ) {
  printErrCode(i);
  return false;
 }
 
 sa=ngadmin_getSwitchTab(nga, &nb);
 displaySwitchTab(sa, nb);
 
 
 return true;
 
}



const struct TreeNode com_scan=COM("scan", do_scan, false, NULL);



