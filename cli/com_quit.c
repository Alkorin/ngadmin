
#include "common.h"




static bool do_quit (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga UNUSED) {
 
 cont=0;
 
 return true;
 
}



const struct TreeNode com_quit=COM("quit", do_quit, false, NULL);



