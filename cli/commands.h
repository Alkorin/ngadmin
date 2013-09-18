
#ifndef DEF_COMMANDS
#define DEF_COMMANDS


#include "common.h"


struct TreeNode {
	const char *name;
	bool (* const comfunc)(int, const char**, struct ngadmin*);
	bool hasArgs;
	const struct TreeNode *sub;
};


#define COM_ROOT_START(v)		const struct TreeNode v = {.name = "<root>", .comfunc = NULL, .hasArgs = false, .sub = (const struct TreeNode[]){
#define COM_ROOT_END			{.name = NULL, .comfunc = NULL, .hasArgs = false, .sub = NULL}}};
#define COM_START(nam)			{.name = #nam, .comfunc = NULL, .hasArgs = false, .sub = (const struct TreeNode[]){
#define COM_END				{.name = NULL, .comfunc = NULL, .hasArgs = false, .sub = NULL}}},
#define COM_TERM(nam, func, args)	{.name = #nam, .comfunc = func, .hasArgs = args, .sub = NULL}, 


extern const struct TreeNode coms;


#endif

