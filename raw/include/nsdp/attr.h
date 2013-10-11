
#ifndef DEF_ATTR
#define DEF_ATTR


#include <stdlib.h>
#include <arpa/inet.h>

#include <nsdp/list.h>



struct attr {
	unsigned short attr;	/* attribute code */
	unsigned short size;	/* attribute size */
	void *data;		/* attribute data */
};



struct attr* newAttr (unsigned short attr, unsigned short size, void *data);


static inline struct attr* newEmptyAttr (unsigned short attr)
{
	return newAttr(attr, 0, NULL);
}


static inline struct attr* newByteAttr (unsigned short attr, unsigned char value)
{
	char *v = malloc(sizeof(char));
	
	*v = value;
	
	return newAttr(attr, sizeof(char), v);
}


static inline struct attr* newShortAttr (unsigned short attr, short value)
{
	short *v = malloc(sizeof(short));
	
	*v = value;
	
	return newAttr(attr, sizeof(short), v);
}


static inline struct attr* newIntAttr (unsigned short attr, int value)
{
	int *v = malloc(sizeof(int));
	
	*v = value;
	
	return newAttr(attr, sizeof(int), v);
}


static inline struct attr* newAddrAttr (unsigned short attr, struct in_addr value)
{
	struct in_addr *v = malloc(sizeof(struct in_addr));
	
	*v = value;
	
	return newAttr(attr, sizeof(struct in_addr), v);
}


void freeAttr (struct attr *at);


void filterAttributes (List *attr, ...);


#endif

