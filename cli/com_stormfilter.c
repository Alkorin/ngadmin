
#include "commands.h"



bool do_stormfilter_enable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_setStormFilterState(nga, 1);
 printErrCode(i);
 
 
 return true;
 
}



bool do_stormfilter_disable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_setStormFilterState(nga, 0);
 printErrCode(i);
 
 
 return true;
 
}



bool do_stormfilter_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i, d=BITRATE_UNSPEC, p, *ports=NULL;
 const struct swi_attr *sa;
 bool ret=true;
 
 
 if ( nb<2 ) {
  printf("Usage: stormfilt set (all <speed0>)|(<port1> <speed1> [<port2> <speed2> ...])\n");
  ret=false;
  goto end;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 ports=malloc(sa->ports*sizeof(int));
 
 if ( strcmp(com[0], "all")==0 ) {
  d=parseBitrate(com[1]);
  com+=2;
  nb-=2;
 }
 
 for (i=0; i<sa->ports; ++i) {
  ports[i]=d;
 }
 
 for (i=0; i<nb; i+=2) {
  if ( (p=strtol(com[i], NULL, 0))<1 || p>sa->ports ) continue;
  ports[p-1]=parseBitrate(com[i+1]);
 }
 
 
 i=ngadmin_setStormFilterValues(nga, ports);
 printErrCode(i);
 
 
 end:
 free(ports);
 
 return ret;
 
}



bool do_stormfilter_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i, s, ret=true, *ports=NULL;
 const struct swi_attr *sa;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 if ( (i=ngadmin_getStormFilterState(nga, &s))!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 
 if ( !s ) {
  printf("storm filter is disabled\n");
  goto end;
 }
 
 printf("storm filter is enabled\n");
 
 ports=malloc(sa->ports*sizeof(int));
 if ( (i=ngadmin_getStormFilterValues(nga, ports))!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 for (i=0; i<sa->ports; ++i) {
  printf("port %i: %s\n", i+1, bitrates[ports[i]]);
 }
 
 end:
 free(ports);
 
 return ret;
 
}



