
#ifndef DEF_COMMANDS
#define DEF_COMMANDS


#include "common.h"


struct TreeNode {
	const char *name;
	bool (* const comfunc)(int, const char**, struct ngadmin*);
	const struct TreeNode *sub;
};


#define COM_ROOT_START(v)	const struct TreeNode v = {.name = "<root>", .comfunc = NULL, .sub = (const struct TreeNode[]){
#define COM_ROOT_END		{.name = NULL, .comfunc = NULL, .sub = NULL}}};
#define COM_START(nam)		{.name = #nam, .comfunc = NULL, .sub = (const struct TreeNode[]){
#define COM_END			{.name = NULL, .comfunc = NULL, .sub = NULL}}},
#define COM_TERM(nam, func)	{.name = #nam, .comfunc = func, .sub = NULL},


extern const struct TreeNode commands;


#endif

