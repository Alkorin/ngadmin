
#include "list.h"




// -------------------------
List* createEmptyList (void) {
 
 List *l;
 
 
 l=malloc(sizeof(List));
 l->first=NULL;
 l->last=NULL;
 l->count=0;
 pthread_cond_init(&l->cond, NULL);
 pthread_mutex_init(&l->mutex, NULL);
 
 
 return l;
 
}



// ------------------------------------------------
void destroyList (List *l, void (*freefunc)(void*)) {
 
 ListNode *pr, *ln;
 
 
 
 if ( l==NULL ) {
  return;
 }
 
 
 pthread_mutex_destroy(&l->mutex);
 pthread_cond_destroy(&l->cond);
 
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
 
 
 pthread_mutex_lock(&l->mutex);
 
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
 
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 
}



// ------------------------------------
void pushBackList (List *l, void* data) {
 
 ListNode *ln;
 
 
 if ( l==NULL ) {
  return;
 }
 
 
 pthread_mutex_lock(&l->mutex);
 
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
 
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 
}



// -------------------------
void* popFrontList (List *l) {
 
 ListNode *ln;
 void* data;
 
 
 if ( l==NULL ) {
  return NULL;
 }
 
 
 pthread_mutex_lock(&l->mutex);
 
 if ( (ln=l->first)==NULL ) {
  pthread_mutex_unlock(&l->mutex);
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
 
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 
 
 return data;
 
}



// ------------------------
void* popBackList (List *l) {
 
 ListNode *ln;
 void* data;
 
 
 
 if ( l==NULL ) {
  return NULL;
 }
 
 
 pthread_mutex_lock(&l->mutex);
 
 if ( (ln=l->last)==NULL ) {
  pthread_mutex_unlock(&l->mutex);
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
 
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 
 
 
 return data;
 
}



// ----------------------------------------------
void clearList (List *l, void (*freefunc)(void*)) {
 
 ListNode *ln, *pr;
 
 
 if ( l==NULL ) {
  return;
 }
 
 
 pthread_mutex_lock(&l->mutex);
 
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
 
 pthread_mutex_unlock(&l->mutex);
 pthread_cond_broadcast(&l->cond);
 
 
}



// ---------------------------------------------------------------
bool findAndDestroy (List *l, void* data, void (*freefunc)(void*)) {
 
 ListNode *ln;
 
 
 
 if ( l==NULL ) {
  return false;
 }
 
 
 pthread_mutex_lock(&l->mutex);
 
 for (ln=l->first; ln!=NULL && ln->data!=data; ln=ln->next);
 
 if ( ln==NULL ) {
  
  pthread_mutex_unlock(&l->mutex);
  
 } else {
  
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
   freefunc(data);
  }
  
  l->count--;
  
  pthread_mutex_unlock(&l->mutex);
  pthread_cond_broadcast(&l->cond);
  
 }
 
 
 
 return true;
 
}



// -------------------------------------------------
void browseList (List *l, void (*browsefunc)(void*)) {
 
 ListNode *ln;
 
 
 
 if ( l==NULL || browsefunc==NULL ) {
  return;
 }
 
 
 pthread_mutex_lock(&l->mutex);
 
 for (ln=l->first; ln!=NULL; ln=ln->next) {
  browsefunc(ln->data);
 }
 
 pthread_mutex_unlock(&l->mutex);
 
 
}


// --------------------------------------
void* convertToArray (List *l, size_t sz) {
 
 ListNode *ln;
 void *tab=NULL, *p;
 
 
 
 if ( l==NULL || sz==0 ) {
  return NULL;
 }
 
 
 pthread_mutex_lock(&l->mutex);
 
 if ( l->count>0 ) {
  
  tab=malloc(l->count*sz);
  p=tab;
  
  for (ln=l->first; ln!=NULL; ln=ln->next) {
   memcpy(p, ln->data, sz);
   p+=sz;
  }
  
 }
 
 pthread_mutex_unlock(&l->mutex);
 
 
 return tab;
 
}




