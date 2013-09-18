
#ifndef DEF_COMMON
#define DEF_COMMON


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <ngadmin.h>


#define UNUSED			__attribute__((unused))
#define NORET			__attribute__((noreturn))


extern int cont;
extern struct termios current_term;


extern const char * const bitrates[], * const prio[];


void displaySwitchTab (const struct swi_attr *sa, int nb);
void printErrCode (int err);
int parseBitrate (const char *s);
char parsePrio (const char *s);


int trim (char *txt, int start);

int explode (const char *commande, char** tab, int maximum);


#endif

