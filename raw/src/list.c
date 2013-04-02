
#include "list.h"




// -------------------------
List* createEmptyList (void) {
 
 List *l;
 
 
 l=malloc(sizeof(List));
 l->first=NULL;
 l->last=NULL;
 l->count=0;
 
 #ifdef MT_SAFE_LIST
 pthread_cond_init(&l->cond, NULL);
 pthread_mutex_init(&l->mutex, NULL);
 #endif
 
 
 return l;
 
}



// ------------------------------------------------
void destroyList (List *l, void (*freefunc)(void*)) {
 
 ListNode *pr, *ln;
 
 
 
 if ( l==NULL ) {
  return;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_destroy(&l->mutex);
 pthread_cond_destroy(&l->cond);
 #endif
 
 for (ln=l->first; ln!=NULL; ) {
  
  pr=ln;
  
  if ( freefunc!=NULL ) {
   freefunc(pr->data);
  }
  
  ln=pr->next;
  free(pr);
  
 }
 
 
 free(l);
 
}



// -------------------------------------
void pushFrontList (List *l, void* data) {
 
 ListNode *ln;
 
 
 if ( l==NULL ) {
  return;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 ln=malloc(sizeof(ListNode));
 ln->data=data;
 ln->prev=NULL;
 ln->next=l->first;
 
 if ( l->first==NULL ) {
  l->last=ln;
 } else {
  l->first->prev=ln;
 }
 
 l->first=ln;
 l->count++;
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 #endif
 
}



// ------------------------------------
void pushBackList (List *l, void* data) {
 
 ListNode *ln;
 
 
 if ( l==NULL ) {
  return;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 ln=malloc(sizeof(ListNode));
 ln->data=data;
 ln->prev=l->last;
 ln->next=NULL;
 
 if ( l->last==NULL ) {
  l->first=ln;
 } else {
  l->last->next=ln;
 }
 
 l->last=ln;
 l->count++;
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 #endif
 
}



// -------------------------
void* popFrontList (List *l) {
 
 ListNode *ln;
 void* data;
 
 
 if ( l==NULL ) {
  return NULL;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 if ( (ln=l->first)==NULL ) {
  #ifdef MT_SAFE_LIST
  pthread_mutex_unlock(&l->mutex);
  #endif
  return NULL;
 }
 
 data=ln->data;
 l->first=ln->next;
 
 if ( ln->next==NULL ) {
  l->last=NULL;
 } else {
  ln->next->prev=NULL;
 }
 
 l->count--;
 free(ln);
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 #endif
 
 
 return data;
 
}



// ------------------------
void* popBackList (List *l) {
 
 ListNode *ln;
 void* data;
 
 
 
 if ( l==NULL ) {
  return NULL;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 if ( (ln=l->last)==NULL ) {
  #ifdef MT_SAFE_LIST
  pthread_mutex_unlock(&l->mutex);
  #endif
  return NULL;
 }
 
 data=ln->data;
 l->last=ln->prev;
 
 if ( ln->prev==NULL ) {
  l->first=NULL;
 } else {
  ln->prev->next=NULL;
 }
 
 l->count--;
 free(ln);
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 #endif
 
 
 return data;
 
}



// ----------------------------------------------
void clearList (List *l, void (*freefunc)(void*)) {
 
 ListNode *ln, *pr;
 
 
 if ( l==NULL ) {
  return;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 for (ln=l->first; ln!=NULL; ) {
  
  pr=ln;
  
  if ( freefunc!=NULL ) {
   freefunc(pr->data);
  }
  
  ln=pr->next;
  free(pr);
  
 }
 
 l->first=NULL;
 l->last=NULL;
 l->count=0;
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 #endif
 
}



static void __destroyElement (List *l, ListNode *ln, void (*freefunc)(void*)) {
 
 
 if ( ln->prev==NULL ) {
  l->first=ln->next;
 } else {
  ln->prev->next=ln->next;
 }
 
 if ( ln->next==NULL ) {
  l->last=ln->prev;
 } else {
  ln->next->prev=ln->prev;
 }
 
 
 if ( freefunc!=NULL ) {
  freefunc(ln->data);
 }
 
 l->count--;
 free(ln);
 
}



// -----------------------------------------------------------------
bool destroyElement (List *l, ListNode *ln, void (*freefunc)(void*)) {
 
 
 if ( l==NULL || ln==NULL ) {
  return false;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 __destroyElement(l, ln, freefunc);
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 #endif
 
 
 return true;
 
}



// ---------------------------------------------------------------
bool findAndDestroy (List *l, void* data, void (*freefunc)(void*)) {
 
 ListNode *ln;
 
 
 
 if ( l==NULL ) {
  return false;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 for (ln=l->first; ln!=NULL && ln->data!=data; ln=ln->next);
 
 if ( ln==NULL ) {
  
  #ifdef MT_SAFE_LIST
  pthread_mutex_unlock(&l->mutex);
  #endif
  
 } else {
  
  __destroyElement(l, ln, freefunc);
  
  #ifdef MT_SAFE_LIST
  pthread_mutex_unlock(&l->mutex);
  pthread_cond_broadcast(&l->cond);
  #endif
  
 }
 
 
 
 return true;
 
}



// -------------------------------------------------
void browseList (List *l, void (*browsefunc)(void*)) {
 
 ListNode *ln;
 
 
 
 if ( l==NULL || browsefunc==NULL ) {
  return;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 for (ln=l->first; ln!=NULL; ln=ln->next) {
  browsefunc(ln->data);
 }
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 #endif
 
}


// --------------------------------------
void* convertToArray (List *l, size_t sz) {
 
 ListNode *ln;
 void *tab=NULL, *p;
 
 
 
 if ( l==NULL || sz==0 ) {
  return NULL;
 }
 
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_lock(&l->mutex);
 #endif
 
 if ( l->count>0 ) {
  
  tab=malloc(l->count*sz);
  p=tab;
  
  for (ln=l->first; ln!=NULL; ln=ln->next) {
   memcpy(p, ln->data, sz);
   p+=sz;
  }
  
 }
 
 #ifdef MT_SAFE_LIST
 pthread_mutex_unlock(&l->mutex);
 #endif
 
 
 return tab;
 
}




