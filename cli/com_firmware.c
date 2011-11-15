
#include "common.h"




static bool do_firmware_show (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 const struct swi_attr *sa;
 bool ret=true;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 puts(sa->firmware);
 
 
 end:
 
 return ret;
 
}



static bool do_firmware_upgrade (const struct TreeNode *tn UNUSED, int nb, const char **com UNUSED, struct ngadmin *nga) {
 
 const struct swi_attr *sa;
 bool ret=true;
 
 
 if ( nb!=1 ) {
  printf("Usage: firmware upgrade <file>\n");
  ret=false;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 printf("not implemented yet\n");
 
 
 end:
 
 return ret;
 
}



static const struct TreeNode com_firmware_show=COM("show", do_firmware_show, false, NULL);
static const struct TreeNode com_firmware_upgrade=COM("upgrade", do_firmware_upgrade, true, NULL);

const struct TreeNode com_firmware=COM("firmware", NULL, false, &com_firmware_show, &com_firmware_upgrade, NULL);



