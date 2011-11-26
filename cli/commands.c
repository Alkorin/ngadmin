
#include "commands.h"




// =============================================================================
// bitrate


// helper function to analyse bitrate speed specifications
static int bitrate_analyse (int nb, const char **com, int *ports) {
 
 int i=0, s;
 
 
 while ( i<nb-1 ) {
  s=parseBitrate(com[i+1]);
  if ( strcmp(com[i], "inout")==0 ) {
   ports[0]=s;
   ports[1]=s;
  } else if ( strcmp(com[i], "in")==0 ) {
   ports[0]=s;
  } else if ( strcmp(com[i], "out")==0 ) {
   ports[1]=s;
  } else {
   break;
  }
  i+=2;
 }
 
 
 return i;
 
}


static bool do_bitrate_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i, k=0, defs[]={12, 12}, p, *ports=NULL;
 const struct swi_attr *sa;
 bool ret=true;
 
 
 if ( nb<2 ) {
  printf("Usage: bitrate set [all SPEEDSPEC] <port1> SPEEDSPEC [<port2> SPEEDSPEC ...]\n");
  printf("SPEEDSPEC: [inout <speed>] [in <ispeed>] [out <ospeed>]\n");
  ret=false;
  goto end;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 ports=malloc(2*sa->ports*sizeof(int));
 
 // get defaults if present
 if ( strcmp(com[k], "all")==0 ) {
  ++k;
  k+=bitrate_analyse(nb-k, &com[k], defs);
 }
 
 // apply defaults
 for (i=0; i<sa->ports; ++i) {
  memcpy(&ports[2*i], defs, sizeof(defs));
 }
 
 // get ports specifics
 while ( k<nb ) {
  p=strtol(com[k++], NULL, 0)-1;
  if ( p>=0 && p<sa->ports ) {
   k+=bitrate_analyse(nb-k, &com[k], &ports[2*p]);
  }
 }
 
 // send it to the switch
 i=ngadmin_setBitrateLimits(nga, ports);
 printErrCode(i);
 
 
 end:
 free(ports);
 
 return ret;
 
}



static bool do_bitrate_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i, ret=true, *ports=NULL;
 const struct swi_attr *sa;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 ports=malloc(2*sa->ports*sizeof(int));
 if ( (i=ngadmin_getBitrateLimits(nga, ports))!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 for (i=0; i<sa->ports; ++i) {
  printf("port %i: in %s, out %s\n", i+1, bitrates[ports[2*i+0]], bitrates[ports[2*i+1]]);
 }
 
 end:
 free(ports);
 
 return ret;
 
}



// =============================================================================
// cabletest


static bool do_cabletest (int nb, const char **com, struct ngadmin *nga) {
 
 bool ret=true;
 const struct swi_attr *sa;
 struct cabletest *ct=NULL;
 int i, j=0, k=0;
 
 
 if ( nb<1 ) {
  printf("Usage: cabletest <port1> [<port2> ...]\n");
  goto end;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 ct=malloc(sa->ports*sizeof(struct cabletest));
 memset(ct, 0, sa->ports*sizeof(struct cabletest));
 
 while ( k<nb ) {
  ct[j].port=strtol(com[k++], NULL, 0);
  if ( ct[j].port>=1 && ct[j].port<=sa->ports ) ++j;
 }
 
 i=ngadmin_cabletest(nga, ct, j);
 if ( i<0 ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 
 for (i=0; i<j; ++i) {
  printf("port %i: %08X %08X\n", ct[i].port, ct[i].v1, ct[i].v2);
 }
 
 
 end:
 free(ct);
 
 return ret;
 
}



// =============================================================================
// defaults


static bool do_defaults (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i, ret=true;
 const struct swi_attr *sa;
 char line[16];
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 printf("The switch settings will be CLEARED. Continue ? [y/N]: ");
 fflush(stdout);
 
 if ( fgets(line, sizeof(line), stdin)!=NULL && strcasecmp(line, "y\n")==0 ) {
  i=ngadmin_defaults(nga);
  printErrCode(i);
 }
 
 
 end:
 
 return ret;
 
}



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
// igmp


static bool do_igmp_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i;
 struct igmp_conf ic;
 
 
 if ( nb!=4 ) {
  printf("Usage: igmp set <enable> <vlan> <validate> <block>\n");
  return false;
 }
 
 if ( ngadmin_getCurrentSwitch(nga)==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 ic.enable=strtol(com[0], NULL, 0);
 ic.vlan=strtol(com[1], NULL, 0);
 ic.validate=strtol(com[2], NULL, 0);
 ic.block=strtol(com[3], NULL, 0);
 
 i=ngadmin_setIGMPConf(nga, &ic);
 printErrCode(i);
 
 
 return true;
 
}



static bool do_igmp_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 const struct swi_attr *sa;
 struct igmp_conf ic;
 bool ret=true;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 i=ngadmin_getIGMPConf(nga, &ic);
 if ( i!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 
 printf("IGMP snooping enabled: %s\n", ic.enable ? "yes" : "no" );
 printf("IGMP snooping vlan: %u\n", ic.vlan);
 printf("Validate IGMPv3 headers: %s\n", ic.validate ? "yes" : "no" );
 printf("Block unknown multicast addresses: %s\n", ic.block ? "yes" : "no" );
 
 
 
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
// mirror


static bool do_mirror_disable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 
 
 if ( ngadmin_getCurrentSwitch(nga)==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 
 i=ngadmin_setMirror(nga, NULL);
 printErrCode(i);
 
 
 return true;
 
}



static bool do_mirror_set (int nb, const char **com, struct ngadmin *nga) {
 
 const struct swi_attr *sa;
 char *ports=NULL;
 bool ret=true;
 int i, k=0;
 
 
 if ( nb<3 ) {
  printf("Usage: mirror set <destination port> clone <port1> [<port2> ...]\n");
  goto end;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 ports=malloc((sa->ports+1)*sizeof(char));
 memset(ports, 0, sa->ports+1);
 
 ports[0]=strtol(com[k++], NULL, 0);
 if ( ports[0]<1 || ports[0]>sa->ports || strcasecmp(com[k++], "clone")!=0 ) {
  printf("syntax error\n");
  ret=false;
  goto end;
 }
 
 
 while ( k<nb ) {
  i=strtol(com[k++], NULL, 0);
  if ( i<1 || i>sa->ports ) {
   printf("port out of range\n");
   ret=false;
   goto end;
  } else if ( i==ports[0] ) {
   printf("destination port cannot be in port list\n");
   ret=false;
   goto end;
  }
  ports[i]=1;
 }
 
 
 i=ngadmin_setMirror(nga, ports);
 printErrCode(i);
 
 
 end:
 free(ports);
 
 return ret;
 
}



static bool do_mirror_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 const struct swi_attr *sa;
 char *ports=NULL;
 int i;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 
 ports=malloc((sa->ports+1)*sizeof(char));
 
 i=ngadmin_getMirror(nga, ports);
 if ( i!=ERR_OK ) {
  printErrCode(i);
  goto end;
 }
 
 if ( ports[0]==0 ) {
  printf("port mirroring is disabled\n");
  goto end;
 }
 
 printf("destination: %i\n", ports[0]);
 printf("ports: ");
 for (i=1; i<=sa->ports; ++i) {
  if ( ports[i] ) {
   printf("%i ", i);
  }
 }
 printf("\n");
 
 
 
 end:
 free(ports);
 
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
// netconf


static bool do_netconf_set (int nb, const char **com, struct ngadmin *nga) {
 
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



// =============================================================================
// password


static bool do_password_change (int nb, const char **com, struct ngadmin *nga) {
 
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



static bool do_ports_statistics_reset (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 
 
 if ( ngadmin_getCurrentSwitch(nga)==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_resetPortsStatistics(nga);
 printErrCode(i);
 
 
 return true;
 
}



static bool do_ports_statistics_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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
 
 printf("Port\tReceived\tSent\tCRC errors\n");
 for (i=0; i<sa->ports; ++i) {
  printf("% 4i%12llu%12llu%14llu\n", i+1, ps[i].recv, ps[i].sent, ps[i].crc);
 }
 
 end:
 free(ps);
 
 
 return ret;
 
}




// =============================================================================
// qos



static bool do_qos_mode (int nb, const char **com, struct ngadmin *nga) {
 
 int i, s, ret=true;
 const struct swi_attr *sa;
 
 
 if ( nb==0 ) {
  printf("Usage: qos mode port|802.1p\n");
  goto end;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 if ( strcasecmp(com[0], "port")==0 ) {
  s=QOS_PORT;
 } else if ( strcasecmp(com[0], "802.1p")==0 ) {
  s=QOS_DOT;
 } else {
  printf("Unknown QOS mode\n");
  ret=false;
  goto end;
 }
 
 
 i=ngadmin_setQOSMode(nga, s);
 printErrCode(i);
 
 
 end:
 
 return ret;
 
}



static bool do_qos_set (int nb, const char **com, struct ngadmin *nga) {
 
 int i, p;
 const struct swi_attr *sa;
 bool ret=true;
 char d=PRIO_UNSPEC, *ports=NULL;
 
 
 if ( nb<2 ) {
  printf("Usage: qos set (all <prio0>)|(<port1> <prio1> [<port2> <prio2> ...])\n");
  ret=false;
  goto end;
 }
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 ports=malloc(sa->ports*sizeof(char));
 
 if ( strcmp(com[0], "all")==0 ) {
  d=parsePrio(com[1]);
  com+=2;
  nb-=2;
 }
 
 for (i=0; i<sa->ports; ++i) {
  ports[i]=d;
 }
 
 for (i=0; i<nb; i+=2) {
  if ( (p=strtol(com[i], NULL, 0))<1 || p>sa->ports ) continue;
  ports[p-1]=parsePrio(com[i+1]);
 }
 
 
 i=ngadmin_setQOSValues(nga, ports);
 printErrCode(i);
 
 
 end:
 free(ports);
 
 return ret;
 
}



static bool do_qos_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i, s=0, ret=true;
 const struct swi_attr *sa;
 char *ports=NULL;
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 if ( (i=ngadmin_getQOSMode(nga, &s))!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 
 printf("QOS mode: ");
 
 if ( s==QOS_DOT ) {
  printf("802.1p\n");
  goto end;
 } else if ( s!=QOS_PORT ) {
  printf("unknown (%i)\n", s);
  goto end;
 }
 
 printf("port based\n");
 
 ports=malloc(sa->ports*sizeof(char));
 
 if ( (i=ngadmin_getQOSValues(nga, ports))!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 for (i=0; i<sa->ports; ++i) {
  printf("port %i: %s\n", i+1, prio[(int)ports[i]]);
 }
 
 
 end:
 free(ports);
 
 return ret;
 
}



// =============================================================================
// quit


static bool do_quit (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga UNUSED) {
 
 cont=0;
 
 return true;
 
}



// =============================================================================
// restart


static bool do_restart (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga UNUSED) {
 
 int i, ret=true;
 const struct swi_attr *sa;
 char line[16];
 
 
 if ( (sa=ngadmin_getCurrentSwitch(nga))==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 
 printf("The switch will be restarted. Continue ? [y/N]: ");
 fflush(stdout);
 
 if ( fgets(line, sizeof(line), stdin)!=NULL && strcasecmp(line, "y\n")==0 ) {
  i=ngadmin_restart(nga);
  printErrCode(i);
 }
 
 
 end:
 
 return ret;
 
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
// stormfilter


static bool do_stormfilter_enable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



static bool do_stormfilter_disable (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



static bool do_stormfilter_set (int nb, const char **com, struct ngadmin *nga) {
 
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



static bool do_stormfilter_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



// =============================================================================
// tree


static void display_node (const struct TreeNode *tn, int depth) {
 
 int i;
 const struct TreeNode *s;
 
 
 for (i=0; i<depth; ++i) {
  putchar('\t');
 }
 
 puts(tn->name);
 
 if ( tn->sub==NULL ) return;
 
 for (s=tn->sub; s->name!=NULL; ++s) {
  display_node(s, depth+1);
 }
 
 
}


static bool do_tree (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga UNUSED) {
 
 
 display_node(&coms, 0);
 
 
 return true;
 
}



// =============================================================================
// vlan


static char vlan_char (int t) {
 
 switch ( t ) {
  case VLAN_TAGGED: return 'T';
  case VLAN_UNTAGGED: return 'U'; 
  case VLAN_NO: return ' ';
  default: return '?';
 }
 
}


static bool print_vlan_pvid (struct ngadmin *nga) {
 
 unsigned short *ports=NULL;
 const struct swi_attr *sa;
 int i;
 bool ret=true;
 
 
 sa=ngadmin_getCurrentSwitch(nga);
 
 ports=malloc(sa->ports*sizeof(unsigned short));
 i=ngadmin_getPVID(nga, ports);
 if ( i!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 
 printf("PVID: \n");
 printf("Port\t");
 for (i=1; i<=sa->ports; ++i) {
  printf("%i\t", i);
 }
 putchar('\n');
 
 printf("VLAN\t");
 for (i=0; i<sa->ports; ++i) {
  printf("%u\t", ports[i]);
 }
 putchar('\n');
 
 
 end:
 free(ports);
 
 return ret;
 
}


static bool print_vlan_dot_adv (struct ngadmin *nga) {
 
 char buffer[512], *b=buffer;
 const struct swi_attr *sa;
 int i, t;
 
 
 sa=ngadmin_getCurrentSwitch(nga);
 
 t=sizeof(buffer);
 i=ngadmin_getVLANDotConf(nga, buffer, &t);
 if ( i!=ERR_OK ) {
  printErrCode(i);
  return false;
 }
 
 printf("Ports configuration: \n");
 printf("VLAN\t");
 for (i=1; i<=sa->ports; ++i) {
  printf("%i\t", i);
 }
 putchar('\n');
 
 while ( b-buffer<t ) {
  printf("%u\t", *(unsigned short*)b);b+=2;
  for (i=1; i<=sa->ports; ++i) {
   printf("%c\t", vlan_char(*b++));
  }
  putchar('\n');
 }
 
 
 return true;
 
}


static bool do_vlan_show (int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i, t, ret=true;
 
 
 if ( ngadmin_getCurrentSwitch(nga)==NULL ) {
  printf("must be logged\n");
  ret=false;
  goto end;
 }
 
 if ( (i=ngadmin_getVLANType(nga, &t))!=ERR_OK ) {
  printErrCode(i);
  ret=false;
  goto end;
 }
 
 
 printf("VLAN type: ");
 switch ( t ) {
  case VLAN_DISABLED: printf("disabled\n"); break;
  case VLAN_PORT_BASIC: printf("port basic\n"); break;
  case VLAN_PORT_ADV: printf("port advanced\n"); break;
  case VLAN_DOT_BASIC: printf("802.1Q basic\n"); break;
  
  case VLAN_DOT_ADV:
   printf("802.1Q advanced\n\n");
   ret=print_vlan_dot_adv(nga);
   putchar('\n');
   ret=print_vlan_pvid(nga);
  break;
  
  default: printf("unknown (%i)\n", t);
 }
 
 
 
 end:
 
 return ret;
 
}



// =============================================================================


COM_ROOT_START(coms)
 
 COM_START(bitrate)
  COM_TERM(set, do_bitrate_set, true)
  COM_TERM(show, do_bitrate_show, false)
 COM_END
 
 COM_TERM(cabletest, do_cabletest, true)
 
 COM_TERM(defaults, do_defaults, false)
 
 COM_START(firmware)
  COM_TERM(show, do_firmware_show, false)
  COM_TERM(upgrade, do_firmware_upgrade, true)
 COM_END
 
 COM_START(igmp)
  COM_TERM(set, do_igmp_set, true)
  COM_TERM(show, do_igmp_show, false)
 COM_END
 
 COM_TERM(list, do_list, false)
 
 COM_TERM(login, do_login, true)
 
 COM_START(mirror)
  COM_TERM(disable, do_mirror_disable, false)
  COM_TERM(set, do_mirror_set, true)
  COM_TERM(show, do_mirror_show, false)
 COM_END
 
 COM_START(name)
  COM_TERM(show, do_name_show, false)
  COM_TERM(set, do_name_set, true)
  COM_TERM(clear, do_name_clear, false)
 COM_END
 
 COM_START(netconf)
  COM_TERM(set, do_netconf_set, true)
 COM_END
 
 COM_START(password)
  COM_TERM(change, do_password_change, true)
  COM_TERM(set, do_password_set, true)
 COM_END
 
 COM_START(ports)
  COM_TERM(state, do_ports_state, false)
  COM_START(statistics)
   COM_TERM(reset, do_ports_statistics_reset, false)
   COM_TERM(show, do_ports_statistics_show, false)
  COM_END
 COM_END
 
 COM_START(qos)
  COM_TERM(mode, do_qos_mode, true)
  COM_TERM(set, do_qos_set, true)
  COM_TERM(show, do_qos_show, false)
 COM_END
 
 COM_TERM(quit, do_quit, false)
 
 COM_TERM(restart, do_restart, false)
 
 COM_TERM(scan, do_scan, false)
 
 COM_START(stormfilter)
  COM_TERM(enable, do_stormfilter_enable, false)
  COM_TERM(disable, do_stormfilter_disable, false)
  COM_TERM(set, do_stormfilter_set, true)
  COM_TERM(show, do_stormfilter_show, false)
 COM_END
 
 COM_TERM(tree, do_tree, false)
 
 COM_START(vlan)
  COM_TERM(show, do_vlan_show, false)
  COM_TERM(mode, NULL, true)
 COM_END
 
COM_ROOT_END



