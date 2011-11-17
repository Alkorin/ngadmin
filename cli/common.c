
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



void printBitrate (int br) {
 
 
 switch ( br ) {
  case BITRATE_UNSPEC: printf("unspecified\n"); break;
  case BITRATE_NOLIMIT: printf("unlimited\n"); break;
  case BITRATE_512K: printf("512K\n"); break;
  case BITRATE_1M: printf("1M\n"); break;
  case BITRATE_2M: printf("2M\n"); break;
  case BITRATE_4M: printf("4M\n"); break;
  case BITRATE_8M: printf("8M\n"); break;
  case BITRATE_16M: printf("16M\n"); break;
  case BITRATE_32M: printf("32M\n"); break;
  case BITRATE_64M: printf("64M\n"); break;
  case BITRATE_128M: printf("128M\n"); break;
  case BITRATE_256M: printf("256M\n"); break;
  case BITRATE_512M: printf("512M\n"); break;
  default: printf("unknown (%i)\n", br);
 }
 
 
}


struct BrStr {
 const char *str;
 int br;
};

static const struct BrStr bitrates[]={
 {"nl", BITRATE_NOLIMIT}, 
 {"512K", BITRATE_512K}, 
 {"1M", BITRATE_1M}, 
 {"2M", BITRATE_2M}, 
 {"4M", BITRATE_4M}, 
 {"8M", BITRATE_8M}, 
 {"16M", BITRATE_16M}, 
 {"32M", BITRATE_32M}, 
 {"64M", BITRATE_64M}, 
 {"128M", BITRATE_128M}, 
 {"256M", BITRATE_256M}, 
 {"512M", BITRATE_512M}, 
 {NULL, BITRATE_UNSPEC}
};


int parseBitrate (const char *s) {
 
 const struct BrStr *b;
 
 
 for (b=bitrates; b->str!=NULL && strcasecmp(b->str, s)!=0; ++b);
 
 
 return b->br;
 
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


