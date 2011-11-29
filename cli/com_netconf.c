
#include "commands.h"



bool do_netconf_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i, k;
 const struct swi_attr *sa;
 struct net_conf nc;
 bool ret=true;
 
 
 if ( nb==0 ) {
  printf("Usage: netconf set [dhcp yes|no] [ip <ip>] [mask <mask>] [gw <gw>]\n");
  return false;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 
 memset(&nc, 0, sizeof(struct net_conf));
 
 for (k=0; k<nb; k+=2) {
  
  if ( strcasecmp(com[k], "dhcp")==0 ) {
   if ( strcasecmp(com[k+1], "yes")==0 ) {
    nc.dhcp=true;
   } else if ( strcasecmp(com[k+1], "no")==0 ) {
    nc.dhcp=false;
   } else {
    printf("Incorrect DHCP value\n");
    ret=false;
    goto end;
   }
   
  } else if ( strcasecmp(com[k], "ip")==0 ) {
   if ( inet_aton(com[k+1], &nc.ip)==0 ) {
    printf("Incorrect IP value\n");
    ret=false;
    goto end;
   }
   
  } else if ( strcasecmp(com[k], "mask")==0 ) {
   if ( inet_aton(com[k+1], &nc.netmask)==0 ) { // TODO: check if it is a correct mask
    printf("Incorrect mask value\n");
    ret=false;
    goto end;
   }
   
  } else if ( strcasecmp(com[k], "gw")==0 ) {
   if ( inet_aton(com[k+1], &nc.gw)==0 ) {
    printf("Incorrect gateway value\n");
    ret=false;
    goto end;
   }
   
  }
  
 }
 
 
 i=ngadmin_setNetConf(nga, &nc);
 if ( i!=ERR_OK ) {
  printErrCode(i);
  ret=false;
 }
 
 
 end:
 
 return ret;
 
}



