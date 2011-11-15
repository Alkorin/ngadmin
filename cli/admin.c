
#include <stdio.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "command.h"
#include "common.h"




#define MAXCOM	8





int cont=1;


static const struct TreeNode rootNode={.sub={
 &com_quit,
 &com_login, 
 &com_scan, 
 &com_ports, 
 &com_password, 
 &com_list, 
 &com_firmware, 
 &com_name, 
 NULL
}};



void printErrCode (int err) {
 
 
 switch ( err ) {
  case ERR_OK: /*printf("ok\n");*/ break;
  case ERR_NET: printf("network error\n"); break;
  case ERR_NOTLOG: printf("no switch selected\n"); break;
  case ERR_BADPASS: printf("wrong password\n"); break;
  case ERR_BADID: printf("bad switch id\n"); break;
  case ERR_INVARG: printf("invalid argument\n"); break;
  case ERR_TIMEOUT: printf("timeout\n"); break;
  default: printf("unknown status code (%i)\n", err);
 }
 
 
}



void displaySwitchTab (const struct swi_attr *sa, int nb) {
 
 int i=0;
 
 
 if ( nb==0 ) {
  printf("no switch found\n");
  return;
 }
 
 
 printf("Num\tMac\t\t\tProduct\t\tName\t\t\tIP/mask\t\t\t\tDHCP\tPorts\tFirmware\n");
 
 for (i=0; i<nb; ++i) {
  printf("%i\t%s\t%s\t\t%s\t%s/", i, ether_ntoa(&sa[i].mac), sa[i].product, sa[i].name, inet_ntoa(sa[i].nc.ip));
  printf("%s\t%s\t%i\t%s\n", inet_ntoa(sa[i].nc.netmask), ( sa[i].nc.dhcp ? "Yes" : "No" ), sa[i].ports, sa[i].firmware);
 }
 
 printf("\nfound %i switch(es)\n", nb);
 
 
}




int main (int argc, char **argv) {
 
 char *line, *com[MAXCOM];
 struct ngadmin *nga=NULL;
 struct timeval tv;
 const struct TreeNode *cur, *next, **tab;
 int i, n;
 
 
 
 if ( argc<2 ) {
  printf("Usage: %s <interface>\n", argv[0]);
  return 1;
 }
 
 memset(com, 0, MAXCOM*sizeof(char*));
 
 if ( (nga=ngadmin_init(argv[1]))==NULL ) {
  fprintf(stderr, "Initialization error\n");
  goto end;
 }
 
 // set timeout
 tv.tv_sec=3;
 tv.tv_usec=0;
 ngadmin_setTimeout(nga, &tv);
 
 
 rl_bind_key('\t', rl_abort); // disable auto completion
 
 
 while ( cont ) {
  
  if ( (line=readline("> "))==NULL ) goto end;
  trim(line, strlen(line));
  if ( *line!=0 ) add_history(line);
  n=explode(line, com, MAXCOM);
  free(line);
  
  
  i=0;
  for (next=&rootNode; i<n; ++i) {
   cur=next;
   for (tab=cur->sub; (next=*tab)!=NULL && strcmp(next->name, com[i])!=0; ++tab);
   if ( next==NULL ) break;
  }
  
  
  if ( i<n ) { // commands left uncompared
   
   if ( cur->hasArgs ) { // left "commands" are in fact parameters
    cur->comfunc(cur, n-i, (const char**)&com[i], nga);
   } else {
    if ( i==0 ) {
     printf("unknown command\n");
    } else {
     printf("unknown %s subcommand\n", com[i-1]);
    }
   }
   
  } else {
   
   cur=next;
   if ( cur->comfunc==NULL ) {
    // print available subcommands
    for (tab=cur->sub; (next=*tab)!=NULL; ++tab) {
     printf("%s ", next->name);
    }
    printf("\n");
    
   } else { // terminal command
   
    cur->comfunc(cur, 0, NULL, nga);
    
   }
  }
  
  /*
  if ( n==0 ) {
   // nothing: do nothing
   
  } else if ( strcmp(com[0], "timeout")==0 ) {
   // 
   
  }
  */
  
  for (i=0; i<MAXCOM; i++) {
   if ( com[i]!=NULL ) {
    free(com[i]);
    com[i]=NULL;
   }
  }
  
 }
 
 
 end:
 ngadmin_close(nga);
 
 
 return 0;
 
}


