
#include "common.h"




static bool do_list (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int n;
 const struct swi_attr *sa;
 
 
 sa=ngadmin_getSwitchTab(nga, &n);
 displaySwitchTab(sa, n);
 
 
 return true;
 
}



const struct TreeNode com_list=COM("list", do_list, false, NULL);



