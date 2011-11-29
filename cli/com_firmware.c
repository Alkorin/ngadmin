
#include "commands.h"



bool do_firmware_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



bool do_firmware_upgrade (int nb, const char **com UNUSED, struct ngadmin *nga) {
 
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



