
#ifndef DEF_COMMON
#define DEF_COMMON


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <nsdp/str.h>
#include <ngadmin.h>


#define UNUSED			__attribute__((unused))
#define NORET			__attribute__((noreturn))


extern int main_loop_continue;
extern struct termios current_term;


void displaySwitchTab (const struct swi_attr *sa, int nb);
void printErrCode (int err);

int explode (const char *commande, char** tab, int maximum);


#endif

