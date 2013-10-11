
#include <stdarg.h>

#include <nsdp/attr.h>
#include <nsdp/protocol.h>



struct attr* newAttr (unsigned short attr, unsigned short size, void *data)
{
	struct attr *at;
	
	
	at = malloc(sizeof(struct attr));
	if (at == NULL)
		return NULL;
	
	at->attr = attr;
	at->size = size;
	at->data = data;
	
	
	return at;
}


void freeAttr (struct attr *at)
{
	if (at != NULL) {
		free(at->data);
		free(at);
	}
}


void filterAttributes (List *attr, ...)
{
	va_list ap;
	ListNode *ln, *pr;
	struct attr *at;
	unsigned short attrcode;
	bool keep;
	
	
	ln = attr->first;
	while (ln != NULL) {
		at = ln->data;
		
		va_start(ap, attr);
		keep = false;
		attrcode = 0;
		while (!keep && attrcode != ATTR_END) {
			attrcode = (unsigned short)va_arg(ap, unsigned int);
			keep = keep || (at->attr == attrcode);
		}
		va_end(ap);
		
		if (keep) {
			ln = ln->next;
		} else {
			pr = ln;
			ln = ln->next;
			destroyElement(attr, pr, (void(*)(void*))freeAttr);
		}
	}
	
}


