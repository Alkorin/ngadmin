
#ifndef DEF_ENCODING_ATTR
#define DEF_ENCODING_ATTR


#include <stdbool.h>

#include <nsdp/attr.h>



struct attr_handler {
	unsigned short attr;					/* attribute code */
	unsigned int size;					/* expected data size */
	bool (*encode)(struct attr *at, unsigned char ports);	/* encode function */
	bool (*decode)(struct attr *at, unsigned char ports);	/* decode function */
};


const struct attr_handler* getAttrHandler (unsigned short attrcode);


#endif

