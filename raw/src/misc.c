
#include <stdlib.h>

#include <nsdp/misc.h>


static const char passwordKey[] = "NtgrSmartSwitchRock";


void passwordEndecode (char *buf, unsigned int len)
{
	const char *k = passwordKey;
	unsigned int i;
	
	if (buf == NULL || len <= 0)
		return;
	
	for (i = 0; i < len; i++) {
		if (*k == '\0')
			k = passwordKey;
		buf[i] ^= *k++;
	}
}


int trim (char *txt, int start)
{
	char *p;
	
	if (txt == NULL)
		return 0;
	
	p = txt + start;
	while (p >= txt && (*p == ' ' || *p == '\n')) {
		*p = '\0';
		p--;
	}
	
	return p - txt + 1;
}


