
#include "common.h"







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




const char* bitrates[]={
 "nl", 
 "512K", 
 "1M", 
 "2M",  
 "4M", 
 "8M", 
 "16M", 
 "32M", 
 "64M", 
 "128M", 
 "256M", 
 "512M", 
 NULL
};


const char* prio[]={
 NULL, 
 "high", 
 "medium", 
 "normal", 
 "low", 
 NULL
};


int parseBitrate (const char *s) {
 
 int i;
 
 
 for (i=0; bitrates[i]!=NULL && strcasecmp(bitrates[i], s)!=0; ++i);
 
 
 return i;
 
}



char parsePrio (const char *s) {
 
 int i;
 
 
 for (i=1; prio[i]!=NULL && strcasecmp(prio[i], s)!=0; ++i);
 
 
 return (char)i;
 
}



void displaySwitchTab (const struct swi_attr *sa, int nb) {
 
 int i=0;
 
 
 if ( nb==0 ) {
  printf("no switch found\n");
  return;
 }
 
 
 printf("Num\tMac\t\t\tProduct\t\tName\t\t\tIP/mask\t\t\tDHCP\tPorts\tFirmware\n");
 
 for (i=0; i<nb; ++i) {
  printf("%i\t%s\t%s\t\t%s\t%s/", i, ether_ntoa(&sa[i].mac), sa[i].product, sa[i].name, inet_ntoa(sa[i].nc.ip));
  printf("%s\t%s\t%i\t%s\n", inet_ntoa(sa[i].nc.netmask), ( sa[i].nc.dhcp ? "Yes" : "No" ), sa[i].ports, sa[i].firmware);
 }
 
 printf("\nfound %i switch(es)\n", nb);
 
 
}



// ----------------------------
int trim (char *txt, int start) {
 
 char *p, c;
 
 
 if ( txt==NULL ) {
  return 0;
 }
 
 //for (p=txt; *p!=0; p++);
 p=txt+start;
 for (p--; p>=txt && ( (c=*p)==' ' || c=='\n' ); *p--=0);
 
 
 return p-txt+1;
 
}



// --------------------------------------------------------
int explode (const char *commande, char** tab, int maximum) {
 
 const char *start, *end;
 char c;
 int n=0, len;
 
 
 for (end=commande; ; n++) {
  
  for (start=end; (c=*start)==' ' && c!=0; start++);
  for (end=start; ( (c=*end)!=' ' || n>=maximum-1 ) && c!=0; end++);
  
  if ( (len=end-start)==0 ) {
   break;
  }
  
  tab[n]=malloc(sizeof(char)*(len+1));
  memcpy(tab[n], start, len);
  tab[n][len]=0;
  
 }
 
 
 return n;
 
}


