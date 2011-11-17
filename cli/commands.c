
#include "commands.h"




// =============================================================================
// firmware


static bool do_firmware_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



static bool do_firmware_upgrade (int nb, const char **com UNUSED, struct ngadmin *nga) {
 
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



// =============================================================================
// list


static bool do_list (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int n;
 const struct swi_attr *sa;
 
 
 sa=ngadmin_getSwitchTab(nga, &n);
 displaySwitchTab(sa, n);
 
 
 return true;
 
}



// =============================================================================
// login


static bool do_login (int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 
 
 if ( nb!=1 ) {
  printf("Usage: login <num>\n");
  return false;
 }
 
 
 i=strtol(com[0], NULL, 0);
 i=ngadmin_login(nga, i);
 printErrCode(i);
 
 
 return true;
 
}



// =============================================================================
// name


static bool do_name_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 const struct swi_attr *sa;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 puts(sa->name);
 
 
 return true;
 
}



static bool do_name_set (int nb, const char **com, struct ngadmin *nga) {
 
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



static bool do_name_clear (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



// =============================================================================
// password


static bool do_password_change (int nb, const char **com, struct ngadmin *nga) {
 
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



static bool do_password_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 
 
 if ( nb!=1 ) {
  printf("Usage: password set <value>\n");
  return false;
 }
 
 i=ngadmin_setPassword(nga, com[0]);
 printErrCode(i);
 
 
 return true;
 
}



// =============================================================================
// ports


static bool do_ports_state (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 unsigned char *ports=NULL;
 bool ret=true;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 ports=malloc(sa->ports*sizeof(unsigned char));
 if ( (i=ngadmin_getPortsStatus(nga, ports))<0 ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 for (i=0; i<sa->ports; i++) {
  printf("port %i: ", i+1);
  switch ( ports[i] ) {
   case 0: printf("down"); break;
   case SPEED_10: printf("up, 10M"); break;
   case SPEED_100: printf("up, 100M"); break;
   case SPEED_1000: printf("up, 1000M"); break;
   default: printf("unknown (%i)", ports[i]);
  }
  putchar('\n');
 }
 
 end:
 free(ports);
 
 
 return ret;
 
}



static bool do_ports_stats_reset (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 
 
 if ( ngadmin_getCurrentSwitch(nga)==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_resetPortsStatistics(nga);
 printErrCode(i);
 
 
 return true;
 
}



static bool do_ports_stats_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 bool ret=true;
 struct port_stats *ps=NULL;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
  
 ps=calloc(sa->ports, sizeof(struct port_stats));
 if ( (i=ngadmin_getPortsStatistics(nga, ps))<0 ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 printf("Port\tReceived\tSent\t\tCRC errors\n");
 for (i=0; i<sa->ports; ++i) {
  printf("%i\t%8llu\t%8llu\t%8llu\n", i+1, ps[i].recv, ps[i].sent, ps[i].crc);
 }
 
 end:
 free(ps);
 
 
 return ret;
 
}



// =============================================================================
// quit


static bool do_quit (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga UNUSED) {
 
 cont=0;
 
 return true;
 
}



// =============================================================================
// scan


static bool do_scan (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



// =============================================================================
// stormfilt


static bool do_stormfilt_enable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



static bool do_stormfilt_disable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



static bool do_stormfilt_reset (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i, *ports=NULL;
 const struct swi_attr *sa;
 bool ret=true;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 ports=malloc(sa->ports*sizeof(int));
 for (i=0; i<sa->ports; ++i) {
  ports[i]=BITRATE_NOLIMIT;
 }
 
 i=ngadmin_setStormFilterValues(nga, ports);
 printErrCode(i);
 
 
 end:
 free(ports);
 
 return ret;
 
}



static bool do_stormfilt_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i, p, *ports=NULL;
 const struct swi_attr *sa;
 bool ret=true;
 
 
 if ( nb==0 ) {
  printf("Usage: stormfilt set <port1> <speed1> [<port2> <speed2> ...]\n");
  ret=false;
  goto end;
 }
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 ports=malloc(sa->ports*sizeof(int));
 for (i=0; i<sa->ports; ++i) {
  ports[i]=BITRATE_UNSPEC;
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



static bool do_stormfilt_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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
  printf("port %i: ", i+1);
  printBitrate(ports[i]);
 }
 
 end:
 free(ports);
 
 return ret;
 
}



// =============================================================================


COM_ROOT_START(coms)
 
 COM_START(firmware)
  COM_TERM(show, do_firmware_show, false)
  COM_TERM(upgrade, do_firmware_upgrade, true)
 COM_END
 
 COM_TERM(list, do_list, false)
 
 COM_TERM(login, do_login, true)
 
 COM_START(name)
  COM_TERM(show, do_name_show, false)
  COM_TERM(set, do_name_set, true)
  COM_TERM(clear, do_name_clear, false)
 COM_END
 
 COM_START(password)
  COM_TERM(change, do_password_change, true)
  COM_TERM(set, do_password_set, true)
 COM_END
 
 COM_START(ports)
  COM_TERM(state, do_ports_state, false)
  COM_START(stats)
   COM_TERM(reset, do_ports_stats_reset, false)
   COM_TERM(show, do_ports_stats_show, false)
  COM_END
 COM_END
 
 COM_TERM(quit, do_quit, false)
 
 COM_TERM(scan, do_scan, false)
 
 COM_START(stormfilt)
  COM_TERM(enable, do_stormfilt_enable, false)
  COM_TERM(disable, do_stormfilt_disable, false)
  COM_TERM(reset, do_stormfilt_reset, false)
  COM_TERM(set, do_stormfilt_set, true)
  COM_TERM(show, do_stormfilt_show, false)
 COM_END
 
COM_ROOT_END



