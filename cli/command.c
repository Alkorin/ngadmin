
#include "command.h"




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


