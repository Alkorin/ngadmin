
#include "common.h"


void printErrCode (int err)
{
	const char *str;
	
	str = ngadmin_errorStr(err);
	
	if (str == NULL)
		printf("unknown status code (%i)\n", err);
	else if (err != ERR_OK)
		puts(str);
}


void displaySwitchTab (const struct swi_attr *sa, int nb)
{
	int i=0;
	
	if (nb == 0) {
		printf("no switch found\n");
		return;
	}
	
	printf("Num\tMac\t\t\tProduct\t\t\tName\t\t\t\t\tIP\t\tPorts\tFirmware\n");
	
	for (i = 0; i < nb; i++)
		printf("% 3d\t%-17s\t%-16.16s\t%-32.32s\t%-15s\t% 5d\t%s\n", i, ether_ntoa(&sa[i].mac), sa[i].product, sa[i].name, inet_ntoa(sa[i].nc.ip), sa[i].ports, sa[i].firmware);
	
	printf("\nfound %i switch(es)\n", nb);
}


int explode (const char *commande, char** tab, int maximum)
{
	const char *start, *end;
	int n = 0, len;
	
	
	for (end = commande; ; n++) {
		for (start = end; *start == ' ' && *start != 0; start++);
		for (end = start; (*end != ' ' || n >= maximum - 1 ) && *end != 0; end++);
		
		len = end - start;
		if (len == 0)
			break;
		
		tab[n] = malloc(sizeof(char) * (len + 1));
		memcpy(tab[n], start, len);
		tab[n][len] = 0;
	}
	
	
	return n;
}


