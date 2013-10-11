
#ifndef DEF_MISC
#define DEF_MISC


void passwordEndecode (char *buf, unsigned int len);


int trim (char *txt, int start);


static inline int min (int a, int b)
{
	return a < b ? a : b;
}


#endif

