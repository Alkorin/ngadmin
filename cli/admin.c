
#include <stdio.h>

#include <getopt.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "common.h"
#include "commands.h"




#define MAXCOM	32


int cont=1;




const struct TreeNode* getSubCom (char **com, int n, int *t) {
 
 int i;
 const struct TreeNode *cur, *next;
 
 
 cur=&coms;
 for (i=0; i<n; ++i) {
  
  // we have reached a terminal command, exit
  if ( cur->sub==NULL ) break;
  
  // search sub command in sub command array
  for (next=cur->sub; next->name!=NULL && strcmp(next->name, com[i])!=0; ++next);
  
  // sub command not found, exit
  if ( next->name==NULL ) {
   next=NULL;
   break;
  }
  
  // next command is now the current one
  cur=next;
  
 }
 
 *t=i;
 
 
 return cur;
 
}




const struct TreeNode *compcur;



char* my_generator (const char* text, int state) {
 
 static int len;
 static const struct TreeNode *tn;
 const char *name;
 
 
 if ( compcur==NULL ) { // sub command not found
  return NULL;
 } else if ( state==0 ) {
  tn=compcur->sub;
  len=strlen(text);
 }
 
 if ( tn==NULL ) { // terminal command
  return NULL;
 }
 
 
 while ( (name=tn->name)!=NULL ) {
  ++tn;
  
  if ( strncmp(name, text, len)==0 ) {
   return strdup(name);
  }
  
 }
 
 
 return NULL;
 
}



char** my_completion (const char *text, int start, int end UNUSED) {
 
 char **matches=NULL;
 char *line, *com[MAXCOM];
 int i, n;
 
 
 
 memset(com, 0, MAXCOM*sizeof(char*));
 line=strdup(rl_line_buffer);
 line[start]=0;
 trim(line, start);
 n=explode(line, com, MAXCOM);
 free(line);
 
 compcur=getSubCom(com, n, &i);
 
 if ( i<n ) compcur=NULL;
 matches=rl_completion_matches(text, my_generator);
 
 
 return matches;
 
}




int main (int argc, char **argv) {
 
 static const struct option opts[]={
  {"keep-broadcasting", no_argument, NULL, 'b'}, 
  {"force-interface", no_argument, NULL, 'f'}, 
  {"interface", required_argument, NULL, 'i'}, 
  {"help", no_argument, NULL, 'h'}, 
  {0, 0, 0, 0}
 };
 char *line, *com[MAXCOM];
 const char *iface="eth0";
 bool kb=false, force=false;
 struct ngadmin *nga=NULL;
 struct timeval tv;
 const struct TreeNode *cur, *next;
 int i, n;
 
 
 
 opterr=0;
 
 while ( (n=getopt_long(argc, argv, "bfi:h", opts, NULL))!=-1 ) {
  switch ( n ) {
   
   case 'b':
    kb=true;
   break;
   
   case 'f':
    force=true;
   break;
   
   case 'i':
    iface=optarg;
   break;
   
   case 'h':
    printf("Usage: %s [-b] [-f] [-i <interface>]\n", argv[0]);
    goto end;
   
   case '?':
    printf("Unknown option: \"%s\"\n", argv[optind-1]);
    goto end;
   
  }
 }
 
 argc-=optind;
 argv+=optind;
 
 
 if ( argc!=0 ) {
  printf("Unknown trailing options\n");
  goto end;
 }
 
 
 memset(com, 0, MAXCOM*sizeof(char*));
 
 if ( (nga=ngadmin_init(iface))==NULL ) {
  fprintf(stderr, "Initialization error\n");
  goto end;
 }
 
 // set timeout
 tv.tv_sec=3;
 tv.tv_usec=0;
 ngadmin_setTimeout(nga, &tv);
 
 
 if ( kb && ngadmin_setKeepBroadcasting(nga, true)!=ERR_OK ) goto end;
 
 if ( force && ngadmin_forceInterface(nga)!=ERR_OK ) goto end;
 
 
 //rl_bind_key('\t', rl_abort); // disable auto completion
 //rl_bind_key('\t', rl_complete); // enable auto-complete
 rl_attempted_completion_function=my_completion;
 rl_completion_entry_function=my_generator;
 
 
 while ( cont ) {
  
  if ( (line=readline("> "))==NULL ) goto end;
  if ( *line!=0 ) add_history(line);
  trim(line, strlen(line));
  n=explode(line, com, MAXCOM);
  free(line);
  
  if ( n==0 ) continue;
  
  cur=getSubCom(com, n, &i);
  
  if ( i<n ) { // commands left unchecked
   
   if ( i==0 ) { // root command
    printf("unknown command\n");
   } else if ( cur->sub!=NULL ) { // intermediate command
    printf("unknown %s subcommand\n", com[i-1]);
   } else if ( !cur->hasArgs ) { // terminal command without arguments
    printf("%s as no subcommand and takes no parameter\n", com[i-1]);
   } else if ( cur->comfunc==NULL ) { // erroneous terminal command without function
    printf("terminal command without function\n");
   } else { // terminal command with arguments, left "commands" are in fact parameters
    cur->comfunc(n-i, (const char**)&com[i], nga);
   }
   
  } else { // no command left
   
   if ( cur->sub!=NULL ) { // intermediate command
    // print available subcommands
    for (next=cur->sub; next->name!=NULL; ++next) {
     printf("%s ", next->name);
    }
    printf("\n");
   } else if ( cur->comfunc==NULL ) { // erroneous terminal command without function
    printf("terminal command without function\n");
   } else { // terminal command without arguments
    cur->comfunc(0, NULL, nga); 
   }
   
  }
  
  
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


