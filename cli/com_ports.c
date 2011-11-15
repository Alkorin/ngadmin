
#include "common.h"




static bool do_ports_state (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



static bool do_ports_stats_show (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
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



static bool do_ports_stats_reset (const struct TreeNode *tn UNUSED, int nb UNUSED, const char **com UNUSED, struct ngadmin *nga) {
 
 int i;
 
 
 if ( ngadmin_getCurrentSwitch(nga)==NULL ) {
  printf("must be logged\n");
  return false;
 }
 
 i=ngadmin_resetPortsStatistics(nga);
 printErrCode(i);
 
 
 return true;
 
}



static const struct TreeNode com_ports_state=COM("state", do_ports_state, false, NULL);

static const struct TreeNode com_ports_stats_show=COM("show", do_ports_stats_show, false, NULL);
static const struct TreeNode com_ports_stats_reset=COM("reset", do_ports_stats_reset, false, NULL);
static const struct TreeNode com_ports_stats=COM("stats", NULL, false, &com_ports_stats_reset, &com_ports_stats_show, NULL);

const struct TreeNode com_ports=COM("ports", NULL, false, &com_ports_state, &com_ports_stats, NULL);



