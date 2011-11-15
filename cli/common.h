
#ifndef DEF_COMMON
#define DEF_COMMON


#include <stdio.h>
#include <stdlib.h>

#include <ngadmin.h>


#define UNUSED				__attribute__((unused))
#define COM(nam, func, args, ...)	{.name=nam, .comfunc=func, .hasArgs=args, .sub={__VA_ARGS__}}



struct TreeNode {
 const char *name;
 bool (* const comfunc)(const struct TreeNode*, int, const char**, struct ngadmin*);
 bool hasArgs;
 const struct TreeNode *sub[];
};



extern int cont;



extern const struct TreeNode com_quit;
extern const struct TreeNode com_login;
extern const struct TreeNode com_scan;
extern const struct TreeNode com_ports;
extern const struct TreeNode com_password;
extern const struct TreeNode com_list;
extern const struct TreeNode com_list;
extern const struct TreeNode com_firmware;
extern const struct TreeNode com_name;



void displaySwitchTab (const struct swi_attr *sa, int nb);
void printErrCode (int err);



#endif

