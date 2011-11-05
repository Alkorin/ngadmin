
#ifndef DEF_LIST
#define DEF_LIST


#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>




typedef struct ListNode ListNode;

struct ListNode {
 void* data;
 ListNode *prev, *next;
};


typedef struct {
 ListNode *first, *last;
 unsigned int count;
 pthread_cond_t cond;
 pthread_mutex_t mutex;
} List;




// Creates an empty list
List* createEmptyList (void);

// Destroys a list, and eventually frees the elements
// NOT MT SAFE
void destroyList (List *l, void (*freefunc)(void*));

// Adds an element at front of the list
void pushFrontList (List *l, void* data);

// Adds an element at back of the list
void pushBackList (List *l, void* data);

// Pops an element from the front of the list and returns its value
void* popFrontList (List *l);

// Pops an element from the back of the list and returns its value
void* popBackList (List *l);

// Clears all the items of the list, and eventually frees them
void clearList (List *l, void (*freefunc)(void*));

// Find and destroy a particular element of the list, and eventually frees it
bool findAndDestroy (List *l, void* data, void (*freefunc)(void*));

// Browse all the items of the list through the callback function
void browseList (List *l, void (*browsefunc)(void*));




#endif

