
#ifndef DEF_COMMON
#define DEF_COMMON


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ngadmin.h>


#define UNUSED				__attribute__((unused))



extern int cont;


extern const char *bitrates[], *prio[];


void displaySwitchTab (const struct swi_attr *sa, int nb);
void printErrCode (int err);
int parseBitrate (const char *s);
char parsePrio (const char *s);


// 
int trim (char *txt, int start);

// 
int explode (const char *commande, char** tab, int maximum);



#endif

